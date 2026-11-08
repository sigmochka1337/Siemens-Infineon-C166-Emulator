#pragma once

#include "interfaces/memory.h"
#include "operands.h"
#include "types.h"

namespace c166 {
class MoveUnit {
 public:
  MoveUnit(IMachine& machine) : machine_(machine) {}

  bool Mov(const Operand* dst, const Operand* src);
  bool Jmpa(ConditionCode condition, const Operand* address);
  bool Jmpi(ConditionCode condition, const Operand* address);
  bool Jmpr(ConditionCode condition, const Operand* displacement);
  bool Jmps(const Operand* address);

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

  bool Jmpa(ConditionCode condition, const CodeAddressOperand* address);
  bool Jmpi(ConditionCode condition, const IndirectMemoryOperand* address);
  bool Jmpr(ConditionCode condition, const RelativeOperand* displacement);
  bool Jmps(const SegmentedAddressOperand* address);

  constexpr void SetFlag(Word& psw, Word flag, bool enabled) noexcept {
    psw = enabled ? psw | flag : psw & ~flag;
  }

  constexpr bool IsWordGpr(RegisterId id) noexcept {
    return id >= RegisterId::kR0 && id <= RegisterId::kR15;
  }

  constexpr bool IsWordRegister(RegisterId id) noexcept {
    return IsWordGpr(id) || (id >= RegisterId::kSP && id <= RegisterId::kSTKUN);
  }

  IMachine& machine_;
};
}  // namespace c166
