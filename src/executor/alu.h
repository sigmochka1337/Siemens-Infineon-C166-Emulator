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
  bool Addb(const Operand* dst, const Operand* src);
  bool Addc(const Operand* dst, const Operand* src);
  bool Addcb(const Operand* dst, const Operand* src);

  bool Sub(const Operand* dst, const Operand* src);
  bool Subb(const Operand* dst, const Operand* src);
  bool Subc(const Operand* dst, const Operand* src);
  bool Subcb(const Operand* dst, const Operand* src);

  bool Neg(const Operand* dst);

 private:
  Word Add(Word left, Word right);
  bool Add(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Add(const RegisterOperand* dst, const RegisterOperand* src);
  bool Add(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Add(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Add(const RegisterOperand* dst, const DirectMemoryOperand* src);

  Byte Addb(Byte left, Byte right);
  bool Addb(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Addb(const RegisterOperand* dst, const RegisterOperand* src);
  bool Addb(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Addb(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Addb(const RegisterOperand* dst, const DirectMemoryOperand* src);

  Word Addc(Word left, Word right);
  bool Addc(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Addc(const RegisterOperand* dst, const RegisterOperand* src);
  bool Addc(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Addc(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Addc(const RegisterOperand* dst, const DirectMemoryOperand* src);

  Byte Addcb(Byte left, Byte right);
  bool Addcb(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Addcb(const RegisterOperand* dst, const RegisterOperand* src);
  bool Addcb(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Addcb(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Addcb(const RegisterOperand* dst, const DirectMemoryOperand* src);

  Word Sub(Word left, Word right);
  bool Sub(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Sub(const RegisterOperand* dst, const RegisterOperand* src);
  bool Sub(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Sub(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Sub(const RegisterOperand* dst, const DirectMemoryOperand* src);

  Byte Subb(Byte left, Byte right);
  bool Subb(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Subb(const RegisterOperand* dst, const RegisterOperand* src);
  bool Subb(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Subb(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Subb(const RegisterOperand* dst, const DirectMemoryOperand* src);

  Word Subc(Word left, Word right);
  bool Subc(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Subc(const RegisterOperand* dst, const RegisterOperand* src);
  bool Subc(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Subc(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Subc(const RegisterOperand* dst, const DirectMemoryOperand* src);

  Byte Subcb(Byte left, Byte right);
  bool Subcb(const RegisterOperand* dst, const ImmediateOperand* src);
  bool Subcb(const RegisterOperand* dst, const RegisterOperand* src);
  bool Subcb(const RegisterOperand* dst, const IndirectMemoryOperand* src);
  bool Subcb(const DirectMemoryOperand* dst, const RegisterOperand* src);
  bool Subcb(const RegisterOperand* dst, const DirectMemoryOperand* src);

  Word Neg(Word value);
  bool Neg(const RegisterOperand* dst);

  constexpr void SetFlag(Word& psw, Word flag, bool enabled) noexcept {
    psw = enabled ? psw | flag : psw & ~flag;
  }

  constexpr bool IsWordGpr(RegisterId id) noexcept {
    return id >= RegisterId::kR0 && id <= RegisterId::kR15;
  }

  constexpr bool IsWordRegister(RegisterId id) noexcept {
    return IsWordGpr(id) || (id >= RegisterId::kSP && id <= RegisterId::kSTKUN);
  }

  constexpr bool IsByteGpr(RegisterId id) noexcept {
    return id >= RegisterId::kRL0 && id <= RegisterId::kRH7;
  }

  constexpr bool IsByteRegister(RegisterId id) noexcept {
    return IsByteGpr(id) || (id >= RegisterId::kSP && id <= RegisterId::kSTKUN);
  }

  IMachine& machine_;
};
}  // namespace c166
