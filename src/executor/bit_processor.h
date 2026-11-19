#pragma once

#include "interfaces/memory.h"
#include "operands.h"

namespace c166 {

class BitProcessor {
 public:
  explicit BitProcessor(IMachine& machine) : machine_(machine) {}

  bool Logical(Mnemonic operation, Byte size, const Operand* dst, const Operand* src);
  bool Complement(Mnemonic operation, const Operand* dst);
  bool Shift(Mnemonic operation, const Operand* dst, const Operand* count);
  bool SetBit(const Operand* dst, bool value);

 private:
  Word LogicalResult(Mnemonic operation, Word left, Word right, bool byte);
  void SetResultFlags(Word result, Word sign_bit, Word extra_flags);

  IMachine& machine_;
};

}  // namespace c166
