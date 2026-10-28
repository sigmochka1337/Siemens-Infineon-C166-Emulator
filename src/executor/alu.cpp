#include "executor/alu.h"

namespace c166 {
bool ALU::Add(const Operand* dst, const Operand* src) {
  if (Add(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Add(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Add(dynamic_cast<const RegisterOperand*>(dst),
          dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Add(dynamic_cast<const DirectMemoryOperand*>(dst),
          dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Add(dynamic_cast<const RegisterOperand*>(dst),
          dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  return false;
}

bool ALU::Add(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  Word left = machine_.ReadRegister(dst->Id());
  Word right = src->Value();
  Word res = Add(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Add(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  Word left = machine_.ReadRegister(dst->Id());
  Word right = machine_.ReadRegister(src->Id());
  Word res = Add(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Add(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) && !IsWordGpr(src->Id())) return false;
  if (src->Mode() != Mode::kPlain || src->Mode() != Mode::kPostIncrement) return false;

  Word left = machine_.ReadRegister(dst->Id());
  const auto r_value = machine_.ReadRegister(src->Id());
  Word right = machine_.Read16(machine_.Resolve(r_value));
  Word res = Add(left, right);
  machine_.WriteRegister(dst->Id(), res);

  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(r_value + 2));
  }
  return true;
}

bool ALU::Add(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(src->Id())) return false;

  const auto l_addr = machine_.Resolve(dst->Address());
  Word left = machine_.Read16(l_addr);
  Word right = machine_.ReadRegister(src->Id());
  Word res = Add(left, right);
  machine_.Write16(l_addr, res);
  return true;
}

bool ALU::Add(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  const auto r_addr = machine_.Resolve(src->Address());
  Word right = machine_.Read16(r_addr);
  Word res = Add(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

Word ALU::Add(Word left, Word right) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  Dword full_sum = static_cast<Dword>(left) + static_cast<Dword>(right);
  Word value = static_cast<Word>(full_sum);
  SetFlag(psw, psw::kE, right == 0x8000);
  SetFlag(psw, psw::kZ, value == 0);
  SetFlag(psw, psw::kV, ((~(left ^ right) & (left ^ value)) & 0x8000) != 0);
  SetFlag(psw, psw::kC, full_sum > 0xFFFF);
  SetFlag(psw, psw::kN, (value & 0x8000) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}
}  // namespace c166
