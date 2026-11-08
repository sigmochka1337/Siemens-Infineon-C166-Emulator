#pragma once
#include "interfaces/memory.h"
#include "operands.h"
#include "types.h"

namespace c166 {
class Stack {
 public:
  Stack(IMachine& machine) : machine_(machine) {}

  bool Push(const Operand* src);
  bool Pop(const Operand* dst);

  bool Calla(ConditionCode condition, const Operand* address);
  bool Calli(ConditionCode condition, const Operand* address);
  bool Callr(const Operand* displacement);
  bool Calls(const Operand* address);
  bool Pcall(const Operand* src, const Operand* address);

  bool Ret();
  bool Rets();
  bool Retp(const Operand* dst);
  bool Reti();

 private:
  bool Push(const RegisterOperand* src);
  bool Pop(const RegisterOperand* dst);

  bool Calla(ConditionCode condition, const CodeAddressOperand* address);
  bool Calli(ConditionCode condition, const IndirectMemoryOperand* address);
  bool Callr(const RelativeOperand* displacement);
  bool Calls(const SegmentedAddressOperand* address);
  bool Pcall(const RegisterOperand* src, const CodeAddressOperand* address);
  bool Retp(const RegisterOperand* dst);

  void PushWord(Word value);
  Word PopWord();
  void SetDataFlags(Word value);

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
