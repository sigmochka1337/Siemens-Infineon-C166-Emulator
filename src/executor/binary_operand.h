#pragma once
#include "interfaces/memory.h"
#include "operands.h"

namespace c166 {
inline bool IsGpr(RegisterId id, bool byte) {
  if (byte) return id >= RegisterId::kRL0 && id <= RegisterId::kRH7;
  return id >= RegisterId::kR0 && id <= RegisterId::kR15;
}

inline bool IsRegister(RegisterId id, bool byte) { return IsGpr(id, byte) || IsShortSfr(id); }

inline Word ReadData(const IMachine& machine, PhysicalAddress address, bool byte) {
  return byte ? machine.Read8(address) : machine.Read16(address);
}

// The compact opcode selects a GPR pair, #data3, [Ri] or [Ri+]. Long forms
// allow a short-addressed register with immediate data or a direct memory word/byte.
inline std::optional<Word> ReadBinarySource(IMachine& machine, const Operand* src, Byte size,
                                            bool byte, bool dst_is_gpr) {
  if (const auto* immediate = dynamic_cast<const ImmediateOperand*>(src)) {
    const Word value = immediate->Value();
    if (size == 2 && dst_is_gpr && value <= 7) return value;
    if (size == 4 && (!byte || value <= 0xFF)) return value;
    return std::nullopt;
  }
  if (const auto* mem = dynamic_cast<const DirectMemoryOperand*>(src); mem && size == 4) {
    return ReadData(machine, machine.ResolveOperand(mem->Address()), byte);
  }
  if (size != 2 || !dst_is_gpr) return std::nullopt;
  if (const auto* reg = dynamic_cast<const RegisterOperand*>(src); reg && IsGpr(reg->Id(), byte)) {
    return machine.ReadShortRegister(reg->Id());
  }
  const auto* indirect = dynamic_cast<const IndirectMemoryOperand*>(src);
  if (!indirect || indirect->Id() < RegisterId::kR0 || indirect->Id() > RegisterId::kR3) {
    return std::nullopt;
  }
  using Mode = IndirectMemoryOperand::AddressMode;
  if (indirect->Mode() != Mode::kPlain && indirect->Mode() != Mode::kPostIncrement) {
    return std::nullopt;
  }
  const Word pointer = machine.ReadRegister(indirect->Id());
  const Word value = ReadData(machine, machine.ResolveOperand(pointer), byte);
  if (indirect->Mode() == Mode::kPostIncrement) {
    machine.WriteRegister(indirect->Id(), static_cast<Word>(pointer + (byte ? 1 : 2)));
  }
  return value;
}

}  // namespace c166
