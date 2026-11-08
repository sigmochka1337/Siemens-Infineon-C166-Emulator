#include "executor/move_unit.h"

#include <cstdint>

namespace c166 {
namespace {

bool IsConditionMet(Word condition, Word psw_value) {
  const bool e = (psw_value & psw::kE) != 0;
  const bool z = (psw_value & psw::kZ) != 0;
  const bool v = (psw_value & psw::kV) != 0;
  const bool c = (psw_value & psw::kC) != 0;
  const bool n = (psw_value & psw::kN) != 0;

  switch (condition) {
    case 0x0:  // cc_UC
      return true;
    case 0x1:  // cc_NET
      return !z && !e;
    case 0x2:  // cc_Z / cc_EQ
      return z;
    case 0x3:  // cc_NZ / cc_NE
      return !z;
    case 0x4:  // cc_V
      return v;
    case 0x5:  // cc_NV
      return !v;
    case 0x6:  // cc_N
      return n;
    case 0x7:  // cc_NN
      return !n;
    case 0x8:  // cc_C / cc_ULT
      return c;
    case 0x9:  // cc_NC / cc_UGE
      return !c;
    case 0xA:  // cc_SGT
      return !z && (n == v);
    case 0xB:  // cc_SLE
      return z || (n != v);
    case 0xC:  // cc_SLT
      return n != v;
    case 0xD:  // cc_SGE
      return n == v;
    case 0xE:  // cc_UGT
      return !z && !c;
    case 0xF:  // cc_ULE
      return z || c;
    default:
      return false;
  }
}

}  // namespace

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

bool MoveUnit::Jmpa(const Operand* condition, const Operand* address) {
  return Jmpa(dynamic_cast<const ImmediateOperand*>(condition),
              dynamic_cast<const DirectMemoryOperand*>(address));
}

bool MoveUnit::Jmpa(const ImmediateOperand* condition, const DirectMemoryOperand* address) {
  if (!condition || !address || condition->Value() > 0xF) return false;
  if (!IsConditionMet(condition->Value(), machine_.ReadRegister(RegisterId::kPSW))) return true;

  CodeAddress pc = machine_.GetPC();
  pc.offset = address->Address();
  machine_.SetPC(pc);
  return true;
}

bool MoveUnit::Jmpi(const Operand* condition, const Operand* address) {
  return Jmpi(dynamic_cast<const ImmediateOperand*>(condition),
              dynamic_cast<const IndirectMemoryOperand*>(address));
}

bool MoveUnit::Jmpi(const ImmediateOperand* condition, const IndirectMemoryOperand* address) {
  if (!condition || !address || condition->Value() > 0xF) return false;
  if (!IsWordGpr(address->Id())) return false;
  if (address->Mode() != IndirectMemoryOperand::AddressMode::kPlain) return false;
  if (!IsConditionMet(condition->Value(), machine_.ReadRegister(RegisterId::kPSW))) return true;

  CodeAddress pc = machine_.GetPC();
  pc.offset = machine_.ReadRegister(address->Id());
  machine_.SetPC(pc);
  return true;
}

bool MoveUnit::Jmpr(const Operand* condition, const Operand* displacement) {
  return Jmpr(dynamic_cast<const ImmediateOperand*>(condition),
              dynamic_cast<const ImmediateOperand*>(displacement));
}

bool MoveUnit::Jmpr(const ImmediateOperand* condition, const ImmediateOperand* displacement) {
  if (!condition || !displacement || condition->Value() > 0xF || displacement->Value() > 0xFF) {
    return false;
  }
  if (!IsConditionMet(condition->Value(), machine_.ReadRegister(RegisterId::kPSW))) return true;

  const auto relative_words = static_cast<std::int8_t>(displacement->Value());
  CodeAddress pc = machine_.GetPC();
  pc.offset = static_cast<Word>(pc.offset + static_cast<std::int16_t>(relative_words) * 2);
  machine_.SetPC(pc);
  return true;
}

bool MoveUnit::Jmps(const Operand* segment, const Operand* address) {
  return Jmps(dynamic_cast<const ImmediateOperand*>(segment),
              dynamic_cast<const DirectMemoryOperand*>(address));
}

bool MoveUnit::Jmps(const ImmediateOperand* segment, const DirectMemoryOperand* address) {
  if (!segment || !address || segment->Value() > 0xFF) return false;

  CodeAddress pc;
  pc.segment = static_cast<Byte>(segment->Value());
  pc.offset = address->Address();
  machine_.SetPC(pc);
  return true;
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
