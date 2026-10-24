#include "executor/executor.h"

#include <optional>
#include <string>
#include <utility>

namespace c166 {
namespace {
ExecutionResult Failure(FaultCode code, const Instruction& instruction, std::string message) {
  Diagnostic diagnostic{code, ToPhysicalAddress(instruction.pc), std::nullopt, std::move(message)};
  return {StepStatus::kFaulted, FlowKind::kSequential, std::move(diagnostic)};
}

ExecutionResult BusFailure(const Diagnostic& diagnostic) {
  return {IsTrap(diagnostic) ? StepStatus::kTrap : StepStatus::kFaulted, FlowKind::kSequential,
          diagnostic};
}

bool IsWordRegister(RegisterId id) {
  return static_cast<Byte>(id) <= static_cast<Byte>(RegisterId::kR15);
}

const RegisterOperand* WordRegister(const Operand& operand) {
  const auto* reg = std::get_if<RegisterOperand>(&operand.value);
  return operand.width == Width::kWord && reg != nullptr && IsWordRegister(reg->id) ? reg : nullptr;
}

bool ConditionHolds(Condition condition, Word flags) {
  const bool n = (flags & psw::kN) != 0;
  const bool c = (flags & psw::kC) != 0;
  const bool v = (flags & psw::kV) != 0;
  const bool z = (flags & psw::kZ) != 0;
  const bool e = (flags & psw::kE) != 0;
  switch (condition) {
    case Condition::kUnconditional:
      return true;
    case Condition::kNotEqualAndNotEnd:
      return !z && !e;
    case Condition::kZero:
      return z;
    case Condition::kNotZero:
      return !z;
    case Condition::kOverflow:
      return v;
    case Condition::kNotOverflow:
      return !v;
    case Condition::kNegative:
      return n;
    case Condition::kNotNegative:
      return !n;
    case Condition::kCarry:
      return c;
    case Condition::kNotCarry:
      return !c;
    case Condition::kSignedGreater:
      return !z && n == v;
    case Condition::kSignedLessOrEqual:
      return z || n != v;
    case Condition::kSignedLess:
      return n != v;
    case Condition::kSignedGreaterOrEqual:
      return n == v;
    case Condition::kUnsignedGreater:
      return !z && !c;
    case Condition::kUnsignedLessOrEqual:
      return z || c;
  }
  return false;
}

Word MoveFlags(Word old_psw, Word value) {
  Word flags = static_cast<Word>(old_psw & ~(psw::kE | psw::kZ | psw::kN));
  if (value == 0x8000) flags |= psw::kE;
  if (value == 0) flags |= psw::kZ;
  if ((value & 0x8000) != 0) flags |= psw::kN;
  return flags;
}

Word AddFlags(Word old_psw, Word left, Word right, Dword sum) {
  const Word value = static_cast<Word>(sum);
  Word flags = static_cast<Word>(old_psw & ~psw::kConditionMask);
  if (right == 0x8000) flags |= psw::kE;
  if (value == 0) flags |= psw::kZ;
  if ((~(left ^ right) & (left ^ value) & 0x8000) != 0) flags |= psw::kV;
  if (sum > 0xFFFF) flags |= psw::kC;
  if ((value & 0x8000) != 0) flags |= psw::kN;
  return flags;
}

void AdvanceExtension(IMachine& machine) {
  auto extension = machine.GetExtension();
  if (extension.remaining == 0) return;
  --extension.remaining;
  if (extension.remaining == 0) extension = {};
  machine.SetExtension(extension);
}
}  // namespace

ExecutionResult Executor::Execute(const Instruction& instruction, IMachine& machine) const {
  if (instruction.pc != machine.GetPC()) {
    return Failure(FaultCode::kInvalidArgument, instruction,
                   "Instruction PC differs from machine PC");
  }
  if ((instruction.pc.offset & 1) != 0 || (instruction.length != 2 && instruction.length != 4)) {
    return Failure(FaultCode::kInvalidEncoding, instruction,
                   "Invalid instruction alignment or length");
  }
  if (static_cast<unsigned>(instruction.pc.offset) + instruction.length > 0x10000u) {
    return Failure(FaultCode::kSegmentBoundary, instruction,
                   "Instruction crosses a code segment boundary (unsupported profile case)");
  }
  if (static_cast<Byte>(instruction.condition) > 0xF || machine.GetExtension().remaining > 4) {
    return Failure(FaultCode::kInvalidArgument, instruction,
                   "Invalid condition or extension count");
  }

  const RegisterOperand* destination = nullptr;
  const RegisterOperand* source_register = nullptr;
  const ImmediateOperand* immediate = nullptr;
  const RelativeOperand* relative = nullptr;
  const auto unsupported = [&]() {
    return Failure(FaultCode::kUnsupportedInstruction, instruction,
                   "Instruction form is outside the initial execution scope");
  };
  switch (instruction.opcode) {
    case Opcode::kNop:
      if (instruction.length != 2 || instruction.operand_count != 0 ||
          instruction.condition != Condition::kUnconditional)
        return unsupported();
      break;
    case Opcode::kMov:
    case Opcode::kAdd:
      if (instruction.operand_count != 2 || instruction.condition != Condition::kUnconditional) {
        return unsupported();
      }
      destination = WordRegister(instruction.operands[0]);
      source_register = WordRegister(instruction.operands[1]);
      if (instruction.operands[1].width == Width::kWord) {
        immediate = std::get_if<ImmediateOperand>(&instruction.operands[1].value);
      }
      if (destination == nullptr || !((instruction.length == 2 && source_register != nullptr) ||
                                      (instruction.opcode == Opcode::kMov &&
                                       instruction.length == 4 && immediate != nullptr))) {
        return unsupported();
      }
      break;
    case Opcode::kJmpR:
      if (instruction.length != 2 || instruction.operand_count != 1 ||
          instruction.operands[0].width != Width::kWord) {
        return unsupported();
      }
      relative = std::get_if<RelativeOperand>(&instruction.operands[0].value);
      if (relative == nullptr) return unsupported();
      break;
    default:
      return unsupported();
  }

  // All operands and boundary calculations are validated before the first write.
  int next_offset = static_cast<int>(instruction.pc.offset) + instruction.length;
  FlowKind flow = FlowKind::kSequential;
  std::optional<Word> new_psw;
  std::optional<Word> value;
  if (relative != nullptr) {
    const auto flags = machine.ReadRegister(RegisterId::kPSW);
    if (!flags.Ok()) return BusFailure(flags.Error());
    if (ConditionHolds(instruction.condition, flags.Value())) {
      next_offset += 2 * static_cast<int>(relative->words);
      flow = FlowKind::kBranch;
    }
  } else if (destination != nullptr) {
    const auto flags = machine.ReadRegister(RegisterId::kPSW);
    if (!flags.Ok()) return BusFailure(flags.Error());
    Word source = immediate != nullptr ? immediate->value : 0;
    if (source_register != nullptr) {
      const auto read = machine.ReadRegister(source_register->id);
      if (!read.Ok()) return BusFailure(read.Error());
      source = read.Value();
    }
    if (instruction.opcode == Opcode::kMov) {
      value = source;
      new_psw = MoveFlags(flags.Value(), source);
    } else {
      const auto left = machine.ReadRegister(destination->id);
      if (!left.Ok()) return BusFailure(left.Error());
      const Dword sum = static_cast<Dword>(left.Value()) + source;
      value = static_cast<Word>(sum);
      new_psw = AddFlags(flags.Value(), left.Value(), source, sum);
    }
  }
  if (next_offset < 0 || next_offset > 0xFFFE) {
    return Failure(FaultCode::kSegmentBoundary, instruction,
                   "Next PC leaves the current code segment (unsupported profile case)");
  }
  if (destination != nullptr) {
    const auto write = machine.WriteRegister(destination->id, value.value());
    if (!write.Ok()) return BusFailure(write.Error());
    const auto flags = machine.WriteRegister(RegisterId::kPSW, new_psw.value());
    if (!flags.Ok()) return BusFailure(flags.Error());
  }
  const auto pc = machine.SetPC({instruction.pc.segment, static_cast<Word>(next_offset)});
  if (!pc.Ok()) return BusFailure(pc.Error());
  AdvanceExtension(machine);
  return {StepStatus::kCompleted, flow, std::nullopt};
}
}  // namespace c166
