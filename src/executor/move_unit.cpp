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
  machine_.WriteRegister(dst->Id(), value);
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(pointer + 2));
  }
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
  machine_.Write16(dst_address, value);
  if (dst->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(dst->Id(), static_cast<Word>(dst_pointer + 2));
  }
  if (src->Mode() == Mode::kPostIncrement) {
    machine_.WriteRegister(src->Id(), static_cast<Word>(src_pointer + 2));
  }
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

}  // namespace c166
