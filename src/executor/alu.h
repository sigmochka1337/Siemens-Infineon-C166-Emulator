#pragma once
#include "exceptions.h"
#include "interfaces/memory.h"
#include "operands.h"
#include "types.h"

namespace c166 {
class ALU {
 public:
  ALU(IMachine& machine) : machine_(machine) {}

  bool Add(const Operand* dst, const Operand* src);

 private:
  Word Add(Word left, Word right);
  bool Add(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Add(const RegisterOperand* dst, const RegisterOperand* src);
  bool Add(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Add(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Add(const RegisterOperand* dst, const DirectMemoryOperand* src);

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
