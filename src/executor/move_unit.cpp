#include "executor/move_unit.h"

namespace c166 {

// C166 Family Instruction Set Manual V2.0, MOV (pp. 95-96), addressing (pp. 135-136).
bool MoveUnit::Mov(const Operand* dst, const Operand* src) {
  if (Mov(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Mov(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Mov(dynamic_cast<const RegisterOperand*>(dst),
          dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Mov(dynamic_cast<const IndirectMemoryOperand*>(dst),
          dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Mov(dynamic_cast<const IndirectMemoryOperand*>(dst),
          dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Mov(dynamic_cast<const IndirectMemoryOperand*>(dst),
          dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  if (Mov(dynamic_cast<const DirectMemoryOperand*>(dst),
          dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Mov(dynamic_cast<const RegisterOperand*>(dst),
          dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  if (Mov(dynamic_cast<const DirectMemoryOperand*>(dst),
          dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  return false;
}

// C166 Family Instruction Set Manual V2.0, MOVB (pp. 97-98).
bool MoveUnit::Movb(const Operand* dst, const Operand* src) {
  if (Movb(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const ImmediateOperand*>(src))) {
    return true;
  }
  if (Movb(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Movb(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Movb(dynamic_cast<const IndirectMemoryOperand*>(dst),
           dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Movb(dynamic_cast<const IndirectMemoryOperand*>(dst),
           dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Movb(dynamic_cast<const IndirectMemoryOperand*>(dst),
           dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  if (Movb(dynamic_cast<const DirectMemoryOperand*>(dst),
           dynamic_cast<const IndirectMemoryOperand*>(src))) {
    return true;
  }
  if (Movb(dynamic_cast<const RegisterOperand*>(dst),
           dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  if (Movb(dynamic_cast<const DirectMemoryOperand*>(dst),
           dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  return false;
}

// C166 Family Instruction Set Manual V2.0, MOVBS (p. 99).
bool MoveUnit::Movbs(const Operand* dst, const Operand* src) {
  if (Movbs(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Movbs(dynamic_cast<const RegisterOperand*>(dst),
            dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  if (Movbs(dynamic_cast<const DirectMemoryOperand*>(dst),
            dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  return false;
}

// C166 Family Instruction Set Manual V2.0, MOVBZ (p. 100).
bool MoveUnit::Movbz(const Operand* dst, const Operand* src) {
  if (Movbz(dynamic_cast<const RegisterOperand*>(dst), dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  if (Movbz(dynamic_cast<const RegisterOperand*>(dst),
            dynamic_cast<const DirectMemoryOperand*>(src))) {
    return true;
  }
  if (Movbz(dynamic_cast<const DirectMemoryOperand*>(dst),
            dynamic_cast<const RegisterOperand*>(src))) {
    return true;
  }
  return false;
}

bool MoveUnit::Mov(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word value = Mov(src->Value());
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Mov(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;

  Word value = Mov(machine_.ReadRegister(src->Id()));
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Mov(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement &&
      src->Mode() != Mode::kDisplacement) {
    return false;
  }

  Word pointer = machine_.ReadRegister(src->Id());
  LogicalAddress address = pointer;
  if (src->Mode() == Mode::kDisplacement) {
    address = static_cast<LogicalAddress>(pointer + src->Displacement());
  }
  Word value = Mov(machine_.Read16(machine_.Resolve(address)));
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(pointer + 2));
  }
  // Explicit GPR writes follow pointer updates (Subsystem manual, p. 3-80).
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Mov(const IndirectMemoryOperand* dst, const RegisterOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;
  if (dst->Mode() != Mode::kPlain && dst->Mode() != Mode::kPreDecrement &&
      dst->Mode() != Mode::kDisplacement) {
    return false;
  }

  // Capture the source before changing an address register that may be the same GPR.
  Word value = machine_.ReadRegister(src->Id());
  LogicalAddress address = machine_.ReadRegister(dst->Id());
  if (dst->Mode() == Mode::kPreDecrement) {
    address = static_cast<LogicalAddress>(address - 2);
    machine_.WriteRegister(dst->Id(), address);
  } else if (dst->Mode() == Mode::kDisplacement) {
    address = static_cast<LogicalAddress>(address + dst->Displacement());
  }
  const auto physical_address = machine_.Resolve(address);
  value = Mov(value);
  machine_.Write16(physical_address, value);
  return true;
}

bool MoveUnit::Mov(const IndirectMemoryOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;
  // Only [Rn], [Rm]; [Rn+], [Rm]; and [Rn], [Rm+] have MOV encodings.
  if (!((dst->Mode() == Mode::kPlain && src->Mode() == Mode::kPlain) ||
        (dst->Mode() == Mode::kPostIncrement && src->Mode() == Mode::kPlain) ||
        (dst->Mode() == Mode::kPlain && src->Mode() == Mode::kPostIncrement))) {
    return false;
  }

  Word dst_pointer = machine_.ReadRegister(dst->Id());
  Word src_pointer = machine_.ReadRegister(src->Id());
  const auto dst_address = machine_.Resolve(dst_pointer);
  const auto src_address = machine_.Resolve(src_pointer);
  Word value = Mov(machine_.Read16(src_address));
  if (dst->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(dst->Id(), static_cast<Word>(dst_pointer + 2));
  }
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(src_pointer + 2));
  }
  machine_.Write16(dst_address, value);
  return true;
}

bool MoveUnit::Mov(const IndirectMemoryOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id())) return false;
  if (dst->Mode() != IndirectMemoryOperand::AddressMode::kPlain) return false;

  const auto dst_address = machine_.Resolve(machine_.ReadRegister(dst->Id()));
  const auto src_address = machine_.Resolve(src->Address());
  Word value = Mov(machine_.Read16(src_address));
  machine_.Write16(dst_address, value);
  return true;
}

bool MoveUnit::Mov(const DirectMemoryOperand* dst, const IndirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(src->Id())) return false;
  if (src->Mode() != IndirectMemoryOperand::AddressMode::kPlain) return false;

  const auto dst_address = machine_.Resolve(dst->Address());
  const auto src_address = machine_.Resolve(machine_.ReadRegister(src->Id()));
  Word value = Mov(machine_.Read16(src_address));
  machine_.Write16(dst_address, value);
  return true;
}

bool MoveUnit::Mov(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word value = Mov(machine_.Read16(machine_.Resolve(src->Address())));
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Mov(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(src->Id())) return false;

  const auto dst_address = machine_.Resolve(dst->Address());
  Word value = Mov(machine_.ReadRegister(src->Id()));
  machine_.Write16(dst_address, value);
  return true;
}

Word MoveUnit::Mov(Word value) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  SetFlag(psw, psw::kE, value == 0x8000);
  SetFlag(psw, psw::kZ, value == 0);
  SetFlag(psw, psw::kN, (value & 0x8000) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

bool MoveUnit::Movb(const RegisterOperand* dst, const ImmediateOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id()) || src->Value() > 0xFF) return false;

  Byte value = Movb(static_cast<Byte>(src->Value()));
  WriteByteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Movb(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || !IsByteGpr(src->Id())) return false;

  Byte value = Movb(static_cast<Byte>(machine_.ReadRegister(src->Id())));
  WriteByteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Movb(const RegisterOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsByteGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;
  if (src->Mode() != Mode::kPlain && src->Mode() != Mode::kPostIncrement &&
      src->Mode() != Mode::kDisplacement) {
    return false;
  }

  Word pointer = machine_.ReadRegister(src->Id());
  LogicalAddress address = pointer;
  if (src->Mode() == Mode::kDisplacement) {
    address = static_cast<LogicalAddress>(pointer + src->Displacement());
  }
  Byte value = Movb(machine_.Read8(machine_.Resolve(address)));
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(pointer + 1));
  }
  // Write last when the byte destination overlaps the address register.
  WriteByteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Movb(const IndirectMemoryOperand* dst, const RegisterOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsByteGpr(src->Id())) return false;
  if (dst->Mode() != Mode::kPlain && dst->Mode() != Mode::kPreDecrement &&
      dst->Mode() != Mode::kDisplacement) {
    return false;
  }

  Byte value = static_cast<Byte>(machine_.ReadRegister(src->Id()));
  LogicalAddress address = machine_.ReadRegister(dst->Id());
  if (dst->Mode() == Mode::kPreDecrement) {
    address = static_cast<LogicalAddress>(address - 1);
    machine_.WriteRegister(dst->Id(), address);
  } else if (dst->Mode() == Mode::kDisplacement) {
    address = static_cast<LogicalAddress>(address + dst->Displacement());
  }
  const auto physical_address = machine_.Resolve(address);
  value = Movb(value);
  machine_.Write8(physical_address, value);
  return true;
}

bool MoveUnit::Movb(const IndirectMemoryOperand* dst, const IndirectMemoryOperand* src) {
  using Mode = IndirectMemoryOperand::AddressMode;
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsWordGpr(src->Id())) return false;
  if (!((dst->Mode() == Mode::kPlain && src->Mode() == Mode::kPlain) ||
        (dst->Mode() == Mode::kPostIncrement && src->Mode() == Mode::kPlain) ||
        (dst->Mode() == Mode::kPlain && src->Mode() == Mode::kPostIncrement))) {
    return false;
  }

  Word dst_pointer = machine_.ReadRegister(dst->Id());
  Word src_pointer = machine_.ReadRegister(src->Id());
  const auto dst_address = machine_.Resolve(dst_pointer);
  const auto src_address = machine_.Resolve(src_pointer);
  Byte value = Movb(machine_.Read8(src_address));
  if (dst->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(dst->Id(), static_cast<Word>(dst_pointer + 1));
  }
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(src_pointer + 1));
  }
  machine_.Write8(dst_address, value);
  return true;
}

bool MoveUnit::Movb(const IndirectMemoryOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id())) return false;
  if (dst->Mode() != IndirectMemoryOperand::AddressMode::kPlain) return false;

  const auto dst_address = machine_.Resolve(machine_.ReadRegister(dst->Id()));
  const auto src_address = machine_.Resolve(src->Address());
  Byte value = Movb(machine_.Read8(src_address));
  machine_.Write8(dst_address, value);
  return true;
}

bool MoveUnit::Movb(const DirectMemoryOperand* dst, const IndirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(src->Id())) return false;
  if (src->Mode() != IndirectMemoryOperand::AddressMode::kPlain) return false;

  const auto dst_address = machine_.Resolve(dst->Address());
  const auto src_address = machine_.Resolve(machine_.ReadRegister(src->Id()));
  Byte value = Movb(machine_.Read8(src_address));
  machine_.Write8(dst_address, value);
  return true;
}

bool MoveUnit::Movb(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(dst->Id())) return false;

  Byte value = Movb(machine_.Read8(machine_.Resolve(src->Address())));
  WriteByteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Movb(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(src->Id())) return false;

  const auto dst_address = machine_.Resolve(dst->Address());
  Byte value = Movb(static_cast<Byte>(machine_.ReadRegister(src->Id())));
  machine_.Write8(dst_address, value);
  return true;
}

Byte MoveUnit::Movb(Byte value) {
  Word psw = machine_.ReadRegister(RegisterId::kPSW);
  SetFlag(psw, psw::kE, value == 0x80);
  SetFlag(psw, psw::kZ, value == 0);
  SetFlag(psw, psw::kN, (value & 0x80) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw);
  return value;
}

bool MoveUnit::Movbs(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsByteGpr(src->Id())) return false;

  Word value = Movbs(static_cast<Byte>(machine_.ReadRegister(src->Id())));
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Movbs(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word value = Movbs(machine_.Read8(machine_.Resolve(src->Address())));
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Movbs(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(src->Id())) return false;

  const auto dst_address = machine_.Resolve(dst->Address());
  Word value = Movbs(static_cast<Byte>(machine_.ReadRegister(src->Id())));
  machine_.Write16(dst_address, value);
  return true;
}

Word MoveUnit::Movbs(Byte value) {
  return Mov(static_cast<Word>((value & 0x80) != 0 ? 0xFF00 | value : value));
}

bool MoveUnit::Movbz(const RegisterOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordGpr(dst->Id()) || !IsByteGpr(src->Id())) return false;

  Word value = Movbz(static_cast<Byte>(machine_.ReadRegister(src->Id())));
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Movbz(const RegisterOperand* dst, const DirectMemoryOperand* src) {
  if (!dst || !src) return false;
  if (!IsWordRegister(dst->Id())) return false;

  Word value = Movbz(machine_.Read8(machine_.Resolve(src->Address())));
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

bool MoveUnit::Movbz(const DirectMemoryOperand* dst, const RegisterOperand* src) {
  if (!dst || !src) return false;
  if (!IsByteRegister(src->Id())) return false;

  const auto dst_address = machine_.Resolve(dst->Address());
  Word value = Movbz(static_cast<Byte>(machine_.ReadRegister(src->Id())));
  machine_.Write16(dst_address, value);
  return true;
}

Word MoveUnit::Movbz(Byte value) { return Mov(static_cast<Word>(value)); }

void MoveUnit::WriteByteRegister(RegisterId id, Byte value) {
  // SFR byte writes clear the other byte (Subsystem manual, pp. 3-5/3-6).
  machine_.WriteRegister(id, value);
}

}  // namespace c166
