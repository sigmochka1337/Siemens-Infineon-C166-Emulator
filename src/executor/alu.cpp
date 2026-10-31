#include "executor/alu.h"

namespace c166 {
// -------------------
// ------- ADD -------
// -------------------
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

// --------------------
// ------- ADDB -------
// --------------------
bool ALU::Addb(const Operand* dst, const Operand* src) {
  if (Addb(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Addb(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Addb(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Addb(dynamic_cast<const DirectMemoryOperand*>(dst),
           dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Addb(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  return false;
}

bool ALU::Addb(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id()) || src->Value() > 0xFF) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  Byte right = static_cast<Byte>(src->Value());
  Byte res = Addb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addb(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || !IsByteGpr(src->Id())) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  Byte right = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  Byte res = Addb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addb(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || src->Id() < RegisterId::kR0 || src->Id() > RegisterId::kR3) {
    return false;
  }
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  const auto r_value = machine_.ReadRegister(src->Id());
  Byte right = machine_.Read8(machine_.Resolve(r_value));
  Byte res = Addb(left, right);
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(r_value + 1));
  }
  // Write the result last in case the destination aliases the address register.
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addb(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(src->Id())) return false;

  const auto l_addr = machine_.Resolve(dst->Address());
  Byte left = machine_.Read8(l_addr);
  Byte right = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  Byte res = Addb(left, right);
  machine_.Write8(l_addr, res);
  return true;
}

bool ALU::Addb(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id())) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  const auto r_addr = machine_.Resolve(src->Address());
  Byte right = machine_.Read8(r_addr);
  Byte res = Addb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

Byte ALU::Addb(Byte left, Byte right) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  Word full_sum = static_cast<Word>(left) + static_cast<Word>(right);
  Byte value = static_cast<Byte>(full_sum);
  SetFlag(psw, psw::kE, right == 0x80);
  SetFlag(psw, psw::kZ, value == 0);
  SetFlag(psw, psw::kV, ((~(left ^ right) & (left ^ value)) & 0x80) != 0);
  SetFlag(psw, psw::kC, full_sum > 0xFF);
  SetFlag(psw, psw::kN, (value & 0x80) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

// --------------------
// ------- ADDC -------
// --------------------
bool ALU::Addc(const Operand* dst, const Operand* src) {
  if (Addc(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Addc(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Addc(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Addc(dynamic_cast<const DirectMemoryOperand*>(dst),
           dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Addc(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  return false;
}

bool ALU::Addc(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  Word right = src->Value();
  Word res = Addc(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addc(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  Word right = machine_.ReadRegister(src->Id());
  Word res = Addc(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addc(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || src->Id() < RegisterId::kR0 || src->Id() > RegisterId::kR3) {
    return false;
  }
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement) return false;

  Word left = machine_.ReadRegister(dst->Id());
  const auto r_value = machine_.ReadRegister(src->Id());
  Word right = machine_.Read16(machine_.Resolve(r_value));
  Word res = Addc(left, right);
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(r_value + 2));
  }
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addc(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(src->Id())) return false;

  const auto l_addr = machine_.Resolve(dst->Address());
  Word left = machine_.Read16(l_addr);
  Word right = machine_.ReadRegister(src->Id());
  Word res = Addc(left, right);
  machine_.Write16(l_addr, res);
  return true;
}

bool ALU::Addc(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  const auto r_addr = machine_.Resolve(src->Address());
  Word right = machine_.Read16(r_addr);
  Word res = Addc(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

Word ALU::Addc(Word left, Word right) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  Dword full_sum = static_cast<Dword>(left) + static_cast<Dword>(right) + ((psw & psw::kC) != 0);
  Word value = static_cast<Word>(full_sum);
  SetFlag(psw, psw::kE, right == 0x8000);
  SetFlag(psw, psw::kZ, value == 0 && (psw & psw::kZ) != 0);
  SetFlag(psw, psw::kV, ((~(left ^ right) & (left ^ value)) & 0x8000) != 0);
  SetFlag(psw, psw::kC, full_sum > 0xFFFF);
  SetFlag(psw, psw::kN, (value & 0x8000) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

// ---------------------
// ------- ADDCB -------
// ---------------------
bool ALU::Addcb(const Operand* dst, const Operand* src) {
  if (Addcb(dynamic_cast<const RegisterOperand*>(dst),
            dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Addcb(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Addcb(dynamic_cast<const RegisterOperand*>(dst),
            dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Addcb(dynamic_cast<const DirectMemoryOperand*>(dst),
            dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Addcb(dynamic_cast<const RegisterOperand*>(dst),
            dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  return false;
}

bool ALU::Addcb(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id()) || src->Value() > 0xFF) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  Byte right = static_cast<Byte>(src->Value());
  Byte res = Addcb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addcb(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || !IsByteGpr(src->Id())) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  Byte right = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  Byte res = Addcb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addcb(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || src->Id() < RegisterId::kR0 || src->Id() > RegisterId::kR3) {
    return false;
  }
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  const auto r_value = machine_.ReadRegister(src->Id());
  Byte right = machine_.Read8(machine_.Resolve(r_value));
  Byte res = Addcb(left, right);
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(r_value + 1));
  }
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Addcb(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(src->Id())) return false;

  const auto l_addr = machine_.Resolve(dst->Address());
  Byte left = machine_.Read8(l_addr);
  Byte right = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  Byte res = Addcb(left, right);
  machine_.Write8(l_addr, res);
  return true;
}

bool ALU::Addcb(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id())) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  const auto r_addr = machine_.Resolve(src->Address());
  Byte right = machine_.Read8(r_addr);
  Byte res = Addcb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

Byte ALU::Addcb(Byte left, Byte right) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  Word full_sum = static_cast<Word>(left) + static_cast<Word>(right) + ((psw & psw::kC) != 0);
  Byte value = static_cast<Byte>(full_sum);
  SetFlag(psw, psw::kE, right == 0x80);
  SetFlag(psw, psw::kZ, value == 0 && (psw & psw::kZ) != 0);
  SetFlag(psw, psw::kV, ((~(left ^ right) & (left ^ value)) & 0x80) != 0);
  SetFlag(psw, psw::kC, full_sum > 0xFF);
  SetFlag(psw, psw::kN, (value & 0x80) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

// -------------------
// ------- SUB -------
// -------------------
// C166 Family Instruction Set Manual V2.0, SUB/SUBB/SUBC/SUBCB (pp. 125-128).
bool ALU::Sub(const Operand* dst, const Operand* src) {
  if (Sub(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Sub(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Sub(dynamic_cast<const RegisterOperand*>(dst),
          dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Sub(dynamic_cast<const DirectMemoryOperand*>(dst),
          dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Sub(dynamic_cast<const RegisterOperand*>(dst),
          dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  return false;
}

bool ALU::Sub(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  Word right = src->Value();
  Word res = Sub(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Sub(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  Word right = machine_.ReadRegister(src->Id());
  Word res = Sub(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Sub(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || src->Id() < RegisterId::kR0 || src->Id() > RegisterId::kR3) {
    return false;
  }
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement) return false;

  Word left = machine_.ReadRegister(dst->Id());
  const auto r_value = machine_.ReadRegister(src->Id());
  Word right = machine_.Read16(machine_.Resolve(r_value));
  Word res = Sub(left, right);
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(r_value + 2));
  }
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Sub(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(src->Id())) return false;

  const auto l_addr = machine_.Resolve(dst->Address());
  Word left = machine_.Read16(l_addr);
  Word right = machine_.ReadRegister(src->Id());
  Word res = Sub(left, right);
  machine_.Write16(l_addr, res);
  return true;
}

bool ALU::Sub(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  const auto r_addr = machine_.Resolve(src->Address());
  Word right = machine_.Read16(r_addr);
  Word res = Sub(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

Word ALU::Sub(Word left, Word right) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  Word value = static_cast<Word>(left - right);
  SetFlag(psw, psw::kE, right == 0x8000);
  SetFlag(psw, psw::kZ, value == 0);
  SetFlag(psw, psw::kV, (((left ^ right) & (left ^ value)) & 0x8000) != 0);
  SetFlag(psw, psw::kC, left < right);
  SetFlag(psw, psw::kN, (value & 0x8000) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

// --------------------
// ------- SUBB -------
// --------------------
bool ALU::Subb(const Operand* dst, const Operand* src) {
  if (Subb(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Subb(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Subb(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Subb(dynamic_cast<const DirectMemoryOperand*>(dst),
           dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Subb(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  return false;
}

bool ALU::Subb(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id()) || src->Value() > 0xFF) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  Byte right = static_cast<Byte>(src->Value());
  Byte res = Subb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subb(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || !IsByteGpr(src->Id())) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  Byte right = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  Byte res = Subb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subb(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || src->Id() < RegisterId::kR0 || src->Id() > RegisterId::kR3) {
    return false;
  }
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  const auto r_value = machine_.ReadRegister(src->Id());
  Byte right = machine_.Read8(machine_.Resolve(r_value));
  Byte res = Subb(left, right);
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(r_value + 1));
  }
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subb(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(src->Id())) return false;

  const auto l_addr = machine_.Resolve(dst->Address());
  Byte left = machine_.Read8(l_addr);
  Byte right = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  Byte res = Subb(left, right);
  machine_.Write8(l_addr, res);
  return true;
}

bool ALU::Subb(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id())) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  const auto r_addr = machine_.Resolve(src->Address());
  Byte right = machine_.Read8(r_addr);
  Byte res = Subb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

Byte ALU::Subb(Byte left, Byte right) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  Byte value = static_cast<Byte>(left - right);
  SetFlag(psw, psw::kE, right == 0x80);
  SetFlag(psw, psw::kZ, value == 0);
  SetFlag(psw, psw::kV, (((left ^ right) & (left ^ value)) & 0x80) != 0);
  SetFlag(psw, psw::kC, left < right);
  SetFlag(psw, psw::kN, (value & 0x80) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

// --------------------
// ------- SUBC -------
// --------------------
bool ALU::Subc(const Operand* dst, const Operand* src) {
  if (Subc(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Subc(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Subc(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Subc(dynamic_cast<const DirectMemoryOperand*>(dst),
           dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Subc(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  return false;
}

bool ALU::Subc(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  Word right = src->Value();
  Word res = Subc(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subc(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  Word right = machine_.ReadRegister(src->Id());
  Word res = Subc(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subc(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || src->Id() < RegisterId::kR0 || src->Id() > RegisterId::kR3) {
    return false;
  }
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement) return false;

  Word left = machine_.ReadRegister(dst->Id());
  const auto r_value = machine_.ReadRegister(src->Id());
  Word right = machine_.Read16(machine_.Resolve(r_value));
  Word res = Subc(left, right);
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(r_value + 2));
  }
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subc(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(src->Id())) return false;

  const auto l_addr = machine_.Resolve(dst->Address());
  Word left = machine_.Read16(l_addr);
  Word right = machine_.ReadRegister(src->Id());
  Word res = Subc(left, right);
  machine_.Write16(l_addr, res);
  return true;
}

bool ALU::Subc(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word left = machine_.ReadRegister(dst->Id());
  const auto r_addr = machine_.Resolve(src->Address());
  Word right = machine_.Read16(r_addr);
  Word res = Subc(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

Word ALU::Subc(Word left, Word right) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  Dword subtrahend = static_cast<Dword>(right) + ((psw & psw::kC) != 0);
  Word value = static_cast<Word>(static_cast<Dword>(left) - subtrahend);
  SetFlag(psw, psw::kE, right == 0x8000);
  SetFlag(psw, psw::kZ, value == 0 && (psw & psw::kZ) != 0);
  SetFlag(psw, psw::kV, (((left ^ right) & (left ^ value)) & 0x8000) != 0);
  SetFlag(psw, psw::kC, static_cast<Dword>(left) < subtrahend);
  SetFlag(psw, psw::kN, (value & 0x8000) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

// ---------------------
// ------- SUBCB -------
// ---------------------
bool ALU::Subcb(const Operand* dst, const Operand* src) {
  if (Subcb(dynamic_cast<const RegisterOperand*>(dst),
            dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Subcb(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Subcb(dynamic_cast<const RegisterOperand*>(dst),
            dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Subcb(dynamic_cast<const DirectMemoryOperand*>(dst),
            dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Subcb(dynamic_cast<const RegisterOperand*>(dst),
            dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  return false;
}

bool ALU::Subcb(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id()) || src->Value() > 0xFF) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  Byte right = static_cast<Byte>(src->Value());
  Byte res = Subcb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subcb(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || !IsByteGpr(src->Id())) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  Byte right = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  Byte res = Subcb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subcb(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || src->Id() < RegisterId::kR0 || src->Id() > RegisterId::kR3) {
    return false;
  }
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  const auto r_value = machine_.ReadRegister(src->Id());
  Byte right = machine_.Read8(machine_.Resolve(r_value));
  Byte res = Subcb(left, right);
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(r_value + 1));
  }
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

bool ALU::Subcb(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(src->Id())) return false;

  const auto l_addr = machine_.Resolve(dst->Address());
  Byte left = machine_.Read8(l_addr);
  Byte right = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  Byte res = Subcb(left, right);
  machine_.Write8(l_addr, res);
  return true;
}

bool ALU::Subcb(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id())) return false;

  Byte left = static_cast<Byte>(machine_.ReadRegister(dst->Id()));
  const auto r_addr = machine_.Resolve(src->Address());
  Byte right = machine_.Read8(r_addr);
  Byte res = Subcb(left, right);
  machine_.WriteRegister(dst->Id(), res);
  return true;
}

Byte ALU::Subcb(Byte left, Byte right) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  Word subtrahend = static_cast<Word>(right) + ((psw & psw::kC) != 0);
  Byte value = static_cast<Byte>(left - subtrahend);
  SetFlag(psw, psw::kE, right == 0x80);
  SetFlag(psw, psw::kZ, value == 0 && (psw & psw::kZ) != 0);
  SetFlag(psw, psw::kV, (((left ^ right) & (left ^ value)) & 0x80) != 0);
  SetFlag(psw, psw::kC, left < subtrahend);
  SetFlag(psw, psw::kN, (value & 0x80) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

// -------------------
// ------- NEG -------
// -------------------
// C166 Family Instruction Set Manual V2.0, NEG (p. 103).
bool ALU::Neg(const Operand* dst) { return Neg(dynamic_cast<const RegisterOperand*>(dst)); }

bool ALU::Neg(const RegisterOperand* dst) {
  if (!dst) return false;
  if (!IsWordGpr(dst->Id())) return false;

  Word value = Neg(machine_.ReadRegister(dst->Id()));
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

Word ALU::Neg(Word value) { return Sub(0, value); }
}  // namespace c166
