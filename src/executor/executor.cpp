#include "executor/executor.h"

#include "exceptions.h"
#include "executor/binary_operand.h"
#include "operands.h"
#include "types.h"

namespace c166 {
namespace {

class Helper {
 public:
  Helper(const Instruction& instruction, IMachine& machine)
      : instruction_(instruction), machine_(machine) {
    // Older decoders represent the short SFR in "reg, #data" by its word address.
    if (instruction.operands.size() == 2 && instruction.size == 4 &&
        dynamic_cast<const ImmediateOperand*>(instruction.operands[1].get())) {
      const auto* dst = dynamic_cast<const DirectMemoryOperand*>(instruction.operands[0].get());
      if (dst && dst->Address() >= 0xFE00 && dst->Address() <= 0xFFDE && !(dst->Address() & 1))
        short_destination_.emplace(DecodeShortRegister((dst->Address() - 0xFE00) / 2, false));
    }
  }

  void ValidateOperandsSize(size_t size) {
    if (instruction_.operands.size() != size) {
      throw InvalidInstruction(instruction_);
    }
  }

  void ValidateFormat(size_t count, Byte bytes) {
    ValidateOperandsSize(count);
    if (instruction_.size != bytes || instruction_.condition.has_value()) {
      throw InvalidInstruction(instruction_);
    }
  }

  const Operand* operator[](size_t index) const {
    if (index == 0 && short_destination_) return &*short_destination_;
    return instruction_.operands[index].get();
  }

  void ValidateBinary(bool byte, bool move, bool compare = false) {
    ValidateOperandsSize(2);
    const auto* dst = dynamic_cast<const RegisterOperand*>((*this)[0]);
    const Operand* src = (*this)[1];
    if (const auto* imm = dynamic_cast<const ImmediateOperand*>(src)) {
      if (instruction_.size == 2) {
        if (!dst || !IsGpr(dst->Id(), byte) || imm->Value() > (move ? 15 : 7))
          throw InvalidInstruction(instruction_);
      } else if (instruction_.size != 4 || (byte && imm->Value() > 255)) {
        throw InvalidInstruction(instruction_);
      }
    } else {
      bool long_form = false;
      for (const auto& operand : instruction_.operands) {
        const auto* indirect = dynamic_cast<const IndirectMemoryOperand*>(operand.get());
        long_form |=
            dynamic_cast<const DirectMemoryOperand*>(operand.get()) != nullptr ||
            (indirect && indirect->Mode() == IndirectMemoryOperand::AddressMode::kDisplacement);
      }
      ValidateFormat(2, long_form ? 4 : 2);
      if (!move && !long_form && (!dst || !IsGpr(dst->Id(), byte))) {
        throw InvalidInstruction(instruction_);
      }
      if (!move && long_form && dst && !dynamic_cast<const DirectMemoryOperand*>(src)) {
        throw InvalidInstruction(instruction_);
      }
    }
    if (instruction_.condition || (compare && !dst)) throw InvalidInstruction(instruction_);
  }

  bool HasStackPointerDestination() const {
    const Operand* destination = (*this)[0];
    if (const auto* reg = dynamic_cast<const RegisterOperand*>(destination)) {
      return reg->Id() == RegisterId::kSP && !machine_.Cpu().sequence.ExtendedRegisters();
    }
    const auto* mem = dynamic_cast<const DirectMemoryOperand*>(destination);
    return mem && machine_.ResolveOperand(mem->Address()) == sfr::kStackPointerAddress;
  }

  ConditionCode Condition() const {
    if (!instruction_.condition || static_cast<unsigned>(*instruction_.condition) > 15) {
      throw InvalidInstruction(instruction_);
    }
    return *instruction_.condition;
  }

  void AdvancePC() {
    CodeAddress pc = machine_.GetPC();
    pc.offset = static_cast<Word>(pc.offset + instruction_.size);
    if ((machine_.ReadRegister(RegisterId::kSYSCON) & syscon::kSegmentationDisabled) != 0) {
      pc.segment = 0;
    }
    machine_.SetPC(pc);
  }

 private:
  std::optional<RegisterOperand> short_destination_;
  const Instruction& instruction_;
  IMachine& machine_;
};
bool IsPrefix(Mnemonic op) {
  return op == Mnemonic::kAtomic || op == Mnemonic::kExtr || op == Mnemonic::kExtp ||
         op == Mnemonic::kExtpr || op == Mnemonic::kExts || op == Mnemonic::kExtsr;
}

void BeginSequence(const Instruction& instruction, Helper& helper, IMachine& machine) {
  const Mnemonic op = instruction.mnemonic;
  const bool address_override = op != Mnemonic::kAtomic && op != Mnemonic::kExtr;
  const size_t count = address_override ? 2 : 1;
  helper.ValidateOperandsSize(count);
  const auto* range = dynamic_cast<const ImmediateOperand*>(helper[count - 1]);
  if (!range || range->Value() < 1 || range->Value() > 4) throw InvalidInstruction(instruction);
  Word address = 0;
  const bool page = op == Mnemonic::kExtp || op == Mnemonic::kExtpr;
  const Word mask = page ? 0x3FF : 0xFF;
  if (!address_override) {
    helper.ValidateFormat(1, 2);
  } else if (const auto* reg = dynamic_cast<const RegisterOperand*>(helper[0])) {
    helper.ValidateFormat(2, 2);
    if (!IsGpr(reg->Id(), false)) throw InvalidInstruction(instruction);
    address = machine.ReadRegister(reg->Id()) & mask;
  } else {
    helper.ValidateFormat(2, 4);
    const auto* imm = dynamic_cast<const ImmediateOperand*>(helper[0]);
    if (!imm || imm->Value() > mask) throw InvalidInstruction(instruction);
    address = imm->Value();
  }
  machine.Cpu().sequence = {op, address, range->Value()};
}

std::array<Word, 4> ReadPages(const IMachine& machine) {
  return {machine.ReadRegister(RegisterId::kDPP0), machine.ReadRegister(RegisterId::kDPP1),
          machine.ReadRegister(RegisterId::kDPP2), machine.ReadRegister(RegisterId::kDPP3)};
}
}  // namespace

void Executor::Execute(const Instruction& instruction) {
  const CodeAddress start = machine_.GetPC();
  auto& cpu = machine_.Cpu();
  const auto pages = ReadPages(machine_);
  Word deferred_flags = 0;
  try {
    deferred_flags = ExecuteInstruction(instruction);
  } catch (const InvalidInstruction&) {
    machine_.SetPC(start);
    throw;
  } catch (const UnknownInstruction&) {
    machine_.SetPC(start);
    throw;
  } catch (const IllegalWordOperandAccess&) {
    stack_.RequestTrap(tfr::kIllegalWordOperand);
  }

  const auto updated = ReadPages(machine_);
  for (size_t i = 0; i < pages.size(); ++i) {
    cpu.previous_pages[i] = pages[i] != updated[i] ? std::optional{pages[i]} : std::nullopt;
  }
  if (!IsPrefix(instruction.mnemonic) && cpu.sequence.remaining && --cpu.sequence.remaining == 0)
    cpu.sequence = {};
  CodeAddress next_pc = machine_.GetPC();
  const bool odd_target = (next_pc.offset & 1u) != 0;
  if (odd_target) {
    stack_.RequestTrap(tfr::kIllegalInstructionAccess);
  }
  stack_.ServiceTraps(deferred_flags);

  // The trap frame saves the odd target. If entry is blocked, IP stays word-aligned.
  if (odd_target && machine_.GetPC() == next_pc) {
    next_pc.offset &= 0xFFFE;
    machine_.SetPC(next_pc);
  }
}

Word Executor::ExecuteInstruction(const Instruction& instruction) {
  Helper helper(instruction, machine_);
  for (const auto& operand : instruction.operands) {
    if (!operand) throw InvalidInstruction(instruction);
  }
  if (instruction.protection_fault) {
    if (instruction.mnemonic != Mnemonic::kSrvwdt && instruction.mnemonic != Mnemonic::kDiswdt &&
        instruction.mnemonic != Mnemonic::kEinit) {
      throw InvalidInstruction(instruction);
    }
    helper.ValidateFormat(0, 4);
    stack_.RequestTrap(tfr::kProtectionFault);
    return 0;
  }
  helper.AdvancePC();

  switch (instruction.mnemonic) {
    case Mnemonic::kNop:
      helper.ValidateFormat(0, 2);
      break;
    case Mnemonic::kEinit:
      helper.ValidateFormat(0, 4);
      machine_.EndInitialization();
      break;
    case Mnemonic::kDiswdt:
      helper.ValidateFormat(0, 4);
      machine_.DisableWatchdog();
      break;
    case Mnemonic::kSrvwdt:
      helper.ValidateFormat(0, 4);
      machine_.ServiceWatchdog();
      break;
    case Mnemonic::kAtomic:
    case Mnemonic::kExtr:
    case Mnemonic::kExts:
    case Mnemonic::kExtsr:
    case Mnemonic::kExtp:
    case Mnemonic::kExtpr:
      BeginSequence(instruction, helper, machine_);
      break;
    case Mnemonic::kTrap: {
      helper.ValidateFormat(1, 2);
      const auto* number = dynamic_cast<const ImmediateOperand*>(helper[0]);
      if (!number || number->Value() > 127) throw InvalidInstruction(instruction);
      stack_.EnterTrap(number->Value() * 4, TrapClass::kSoftware);
      break;
    }
    case Mnemonic::kCmp:
    case Mnemonic::kCmpB: {
      const bool byte = instruction.mnemonic == Mnemonic::kCmpB;
      helper.ValidateBinary(byte, false, true);
      if (alu_.Compare(byte, instruction.size, helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }
    case Mnemonic::kNegB:
      helper.ValidateFormat(1, 2);
      if (alu_.Negb(helper[0])) break;
      throw InvalidInstruction(instruction);
    case Mnemonic::kMul:
    case Mnemonic::kMulU:
      helper.ValidateFormat(2, 2);
      if (alu_.Multiply(instruction.mnemonic == Mnemonic::kMul, helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    case Mnemonic::kDiv:
    case Mnemonic::kDivL:
    case Mnemonic::kDivU:
    case Mnemonic::kDivLU:
      helper.ValidateFormat(1, 2);
      if (alu_.Divide(instruction.mnemonic, helper[0])) break;
      throw InvalidInstruction(instruction);
    case Mnemonic::kMov: {
      helper.ValidateBinary(false, true);

      if (move_.Mov(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kMovB: {
      helper.ValidateBinary(true, true);

      if (move_.Movb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kMovBS: {
      helper.ValidateBinary(false, true);

      if (move_.Movbs(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kMovBZ: {
      helper.ValidateBinary(false, true);

      if (move_.Movbz(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAdd: {
      helper.ValidateBinary(false, false);

      if (!alu_.Add(helper[0], helper[1])) {
        throw InvalidInstruction(instruction);
      }
      if (helper.HasStackPointerDestination()) {
        return stack_.CheckUnderflow(machine_.ReadRegister(RegisterId::kSP));
      }
      break;
    }

    case Mnemonic::kAddB: {
      helper.ValidateBinary(true, false);

      if (alu_.Addb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAddC: {
      helper.ValidateBinary(false, false);

      if (alu_.Addc(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAddCB: {
      helper.ValidateBinary(true, false);

      if (alu_.Addcb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kSub: {
      helper.ValidateBinary(false, false);

      if (!alu_.Sub(helper[0], helper[1])) {
        throw InvalidInstruction(instruction);
      }
      if (helper.HasStackPointerDestination()) {
        return stack_.CheckOverflow(machine_.ReadRegister(RegisterId::kSP));
      }
      break;
    }

    case Mnemonic::kSubB: {
      helper.ValidateBinary(true, false);

      if (alu_.Subb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kSubC: {
      helper.ValidateBinary(false, false);

      if (alu_.Subc(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kSubCB: {
      helper.ValidateBinary(true, false);

      if (alu_.Subcb(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kNeg: {
      helper.ValidateFormat(1, 2);

      if (alu_.Neg(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAnd:
    case Mnemonic::kAndB:
    case Mnemonic::kOr:
    case Mnemonic::kOrB:
    case Mnemonic::kXor:
    case Mnemonic::kXorB: {
      const bool byte = instruction.mnemonic == Mnemonic::kAndB ||
                        instruction.mnemonic == Mnemonic::kOrB ||
                        instruction.mnemonic == Mnemonic::kXorB;
      helper.ValidateBinary(byte, false);
      if (bits_.Logical(instruction.mnemonic, instruction.size, helper[0], helper[1])) {
        break;
      }
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCpl:
    case Mnemonic::kCplB: {
      helper.ValidateFormat(1, 2);
      if (bits_.Complement(instruction.mnemonic, helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kShl:
    case Mnemonic::kShr:
    case Mnemonic::kAshr:
    case Mnemonic::kRol:
    case Mnemonic::kRor: {
      helper.ValidateFormat(2, 2);
      if (bits_.Shift(instruction.mnemonic, helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kBSet:
    case Mnemonic::kBClr: {
      helper.ValidateFormat(1, 2);
      if (bits_.SetBit(helper[0], instruction.mnemonic == Mnemonic::kBSet)) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kBfldl:
    case Mnemonic::kBfldh:
      helper.ValidateFormat(3, 4);
      if (bits_.ModifyBitField(instruction.mnemonic == Mnemonic::kBfldh, helper[0], helper[1],
                               helper[2])) {
        break;
      }
      throw InvalidInstruction(instruction);

    case Mnemonic::kPush: {
      helper.ValidateFormat(1, 2);

      if (stack_.Push(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kPop: {
      helper.ValidateFormat(1, 2);

      if (stack_.Pop(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCallA: {
      helper.ValidateOperandsSize(1);
      if (instruction.size != 4) throw InvalidInstruction(instruction);

      if (stack_.Calla(helper.Condition(), helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCallI: {
      helper.ValidateOperandsSize(1);
      if (instruction.size != 2) throw InvalidInstruction(instruction);

      if (stack_.Calli(helper.Condition(), helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCallR: {
      helper.ValidateFormat(1, 2);

      if (stack_.Callr(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kCallS: {
      helper.ValidateFormat(1, 4);

      if (stack_.Calls(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kPCall: {
      helper.ValidateFormat(2, 4);

      if (stack_.Pcall(helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJmpA: {
      helper.ValidateOperandsSize(1);
      if (instruction.size != 4) throw InvalidInstruction(instruction);

      if (jump_.Jmpa(helper.Condition(), helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJmpI: {
      helper.ValidateOperandsSize(1);
      if (instruction.size != 2) throw InvalidInstruction(instruction);

      if (jump_.Jmpi(helper.Condition(), helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJmpR: {
      helper.ValidateOperandsSize(1);
      if (instruction.size != 2) throw InvalidInstruction(instruction);

      if (jump_.Jmpr(helper.Condition(), helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJmpS: {
      helper.ValidateFormat(1, 4);

      if (jump_.Jmps(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kJb:
    case Mnemonic::kJnb:
    case Mnemonic::kJbc:
    case Mnemonic::kJnbs: {
      helper.ValidateFormat(2, 4);
      if (jump_.BranchOnBit(instruction.mnemonic, helper[0], helper[1])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kRet: {
      helper.ValidateFormat(0, 2);

      if (stack_.Ret()) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kRetS: {
      helper.ValidateFormat(0, 2);

      if (stack_.Rets()) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kRetP: {
      helper.ValidateFormat(1, 2);

      if (stack_.Retp(helper[0])) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kReti: {
      helper.ValidateFormat(0, 2);

      if (stack_.Reti()) break;
      throw InvalidInstruction(instruction);
    }

    default:
      throw UnknownInstruction(instruction);
  }
  return 0;
}
}  // namespace c166
