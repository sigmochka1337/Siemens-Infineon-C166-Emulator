#include "executor.h"

#include "exceptions.h"
#include "operands.h"
#include "types.h"

namespace c166 {
namespace {

constexpr void SetFlag(Word& psw, Word flag, bool enabled) noexcept {
  psw = enabled ? psw | flag : psw & ~flag;
}

constexpr bool IsWordGpr(RegisterId id) noexcept {
  return id >= RegisterId::kR0 && id <= RegisterId::kR15;
}

constexpr bool IsWordRegister(RegisterId id) noexcept {
  return IsWordGpr(id) || (id >= RegisterId::kSP && id <= RegisterId::kSTKUN);
}

Word Add(Word left, Word right, Word& psw) {
  Dword full_sum = static_cast<Dword>(left) + static_cast<Dword>(right);
  Word value = static_cast<Word>(full_sum);

  SetFlag(psw, psw::kE, right == 0x8000);
  SetFlag(psw, psw::kZ, value == 0);
  SetFlag(psw, psw::kV, ((~(left ^ right) & (left ^ value)) & 0x8000) != 0);
  SetFlag(psw, psw::kC, full_sum > 0xFFFF);
  SetFlag(psw, psw::kN, (value & 0x8000) != 0);

  return value;
}

bool Add(const RegisterOperand* dst, const ImmediateOperand* src, IMachine& machine) {
  if (!IsWordRegister(dst->Id())) {
    return false;
  }
  Word psw = machine.ReadRegister(RegisterId::kPSW);
  Word left = machine.ReadRegister(dst->Id());
  Word right = src->Value();
  Word res = Add(left, right, psw);
  machine.WriteRegister(RegisterId::kPSW, psw);
  machine.WriteRegister(dst->Id(), res);
  return true;
}

bool Add(const RegisterOperand* dst, const RegisterOperand* src, IMachine& machine) {
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) {
    return false;
  }
  Word psw = machine.ReadRegister(RegisterId::kPSW);
  Word left = machine.ReadRegister(dst->Id());
  Word right = machine.ReadRegister(src->Id());
  Word res = Add(left, right, psw);
  machine.WriteRegister(RegisterId::kPSW, psw);
  machine.WriteRegister(dst->Id(), res);
  return true;
}

bool Add(const RegisterOperand* dst, const IndirectMemoryOperand* src, IMachine& machine) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) {
    return false;
  }
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement) {
    return false;
  }

  Word psw = machine.ReadRegister(RegisterId::kPSW);
  Word left = machine.ReadRegister(dst->Id());
  const auto r_addr = machine.Resolve(machine.ReadRegister(src->Id()));
  Word right = machine.Read16(r_addr);
  Word res = Add(left, right, psw);
  machine.WriteRegister(RegisterId::kPSW, psw);
  machine.WriteRegister(dst->Id(), res);

  if (src->Mode() == Mode::kPostIncrement) {
    machine.WriteRegister(src->Id(), static_cast<Word>(machine.ReadRegister(src->Id()) + 2));
  }
  return true;
}

bool Add(const DirectMemoryOperand* dst, const RegisterOperand* src, IMachine& machine) {
  if (!IsWordRegister(src->Id())) {
    return false;
  }

  Word psw = machine.ReadRegister(RegisterId::kPSW);
  const auto l_addr = machine.Resolve(dst->Address());
  Word left = machine.Read16(l_addr);
  Word right = machine.ReadRegister(src->Id());
  Word res = Add(left, right, psw);
  machine.WriteRegister(RegisterId::kPSW, psw);
  machine.Write16(l_addr, res);
  return true;
}

bool Add(const RegisterOperand* dst, const DirectMemoryOperand* src, IMachine& machine) {
  if (!IsWordRegister(dst->Id())) {
    return false;
  }

  Word psw = machine.ReadRegister(RegisterId::kPSW);
  Word left = machine.ReadRegister(dst->Id());
  const auto r_addr = machine.Resolve(src->Address());
  Word right = machine.Read16(r_addr);
  Word res = Add(left, right, psw);
  machine.WriteRegister(RegisterId::kPSW, psw);
  machine.WriteRegister(dst->Id(), res);
  return true;
}
}  // namespace

void Executor::Execute(const Instruction& instruction) {
  switch (instruction.mnemonic) {
    case Mnemonic::kAdd: {
      if (instruction.operands.size() != 2) {
        throw InvalidInstruction(instruction);
      }
      const auto* dst = instruction.operands[0].get();
      const auto* src = instruction.operands[1].get();

      const auto* dst_reg = dynamic_cast<const RegisterOperand*>(dst);
      const auto* dst_mem = dynamic_cast<const DirectMemoryOperand*>(dst);

      const auto* src_reg = dynamic_cast<const RegisterOperand*>(src);
      const auto* src_imm = dynamic_cast<const ImmediateOperand*>(src);
      const auto* src_mem = dynamic_cast<const DirectMemoryOperand*>(src);
      const auto* src_ind = dynamic_cast<const IndirectMemoryOperand*>(src);

      // ADD reg, #data
      if (dst_reg && src_imm && Add(dst_reg, src_imm, machine_)) {
        break;
      }

      // ADD Rw_n Rw_m
      if (dst_reg && src_reg && Add(dst_reg, src_reg, machine_)) {
        break;
      }

      // ADD Rw_n [Rw_i]
      // ADD Rw_n [Rw_i+]
      if (dst_reg && src_ind && Add(dst_reg, src_ind, machine_)) {
        break;
      }

      // ADD mem reg
      if (dst_mem && src_reg && Add(dst_mem, src_reg, machine_)) {
        break;
      }

      // ADD reg, mem
      if (dst_reg && src_mem && Add(dst_reg, src_mem, machine_)) {
        break;
      }
      throw InvalidInstruction(instruction);
    }

    default:
      throw UnknownInstruction(instruction);
  }
}
}  // namespace c166
