#pragma once

#include "interfaces/memory.h"
#include "operands.h"
#include "types.h"

namespace c166 {
class MoveUnit {
 public:
  MoveUnit(IMachine& machine) : machine_(machine) {}

  bool Mov(const Operand* dst, const Operand* src);
  bool Movb(const Operand* dst, const Operand* src);
  bool Movbs(const Operand* dst, const Operand* src);
  bool Movbz(const Operand* dst, const Operand* src);

 private:
  Word Mov(Word value);
  bool Mov(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Mov(const RegisterOperand* dst, const RegisterOperand* src);
  bool Mov(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Mov(const IndirectMemoryOperand* dst, const RegisterOperand* src);
  bool Mov(const IndirectMemoryOperand* dst, const IndirectMemoryOperand* src);
  bool Mov(const IndirectMemoryOperand* dst, const DirectMemoryOperand* src);
  bool Mov(const DirectMemoryOperand* dst, const IndirectMemoryOperand* src);
  bool Mov(const RegisterOperand* dst, const DirectMemoryOperand* src);
  bool Mov(const DirectMemoryOperand* dst, const RegisterOperand* src);

  Byte Movb(Byte value);
  bool Movb(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Movb(const RegisterOperand* dst, const RegisterOperand* src);
  bool Movb(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Movb(const IndirectMemoryOperand* dst, const RegisterOperand* src);
  bool Movb(const IndirectMemoryOperand* dst, const IndirectMemoryOperand* src);
  bool Movb(const IndirectMemoryOperand* dst, const DirectMemoryOperand* src);
  bool Movb(const DirectMemoryOperand* dst, const IndirectMemoryOperand* src);
  bool Movb(const RegisterOperand* dst, const DirectMemoryOperand* src);
  bool Movb(const DirectMemoryOperand* dst, const RegisterOperand* src);

  Word Movbs(Byte value);
  bool Movbs(const RegisterOperand* dst, const RegisterOperand* src);
  bool Movbs(const RegisterOperand* dst, const DirectMemoryOperand* src);
  bool Movbs(const DirectMemoryOperand* dst, const RegisterOperand* src);

  Word Movbz(Byte value);
  bool Movbz(const RegisterOperand* dst, const RegisterOperand* src);
  bool Movbz(const RegisterOperand* dst, const DirectMemoryOperand* src);
  bool Movbz(const DirectMemoryOperand* dst, const RegisterOperand* src);

  constexpr void SetFlag(Word& psw, Word flag, bool enabled) noexcept {
    psw = enabled ? psw | flag : psw & ~flag;
  }

  constexpr bool IsWordGpr(RegisterId id) noexcept {
    return id >= RegisterId::kR0 && id <= RegisterId::kR15;
  }

  constexpr bool IsWordRegister(RegisterId id) noexcept {
    return IsWordGpr(id) || (id >= RegisterId::kSP && id <= RegisterId::kTFR);
  }

  constexpr bool IsByteGpr(RegisterId id) noexcept {
    return id >= RegisterId::kRL0 && id <= RegisterId::kRH7;
  }

  constexpr bool IsByteRegister(RegisterId id) noexcept {
    return IsByteGpr(id) || (id >= RegisterId::kSP && id <= RegisterId::kTFR);
  }

  void WriteByteRegister(RegisterId id, Byte value);

  IMachine& machine_;
};
}  // namespace c166
