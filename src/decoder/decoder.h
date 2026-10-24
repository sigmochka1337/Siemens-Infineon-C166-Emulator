#pragma once

#include "interfaces/decoder.h"

namespace c166 {
// Minimal verified forms; see docs/isa_scope.md before extending the table.
class Decoder final : public IDecoder {
 public:
  Result<Byte> InstructionLength(const std::array<Byte, 2>& first_word) const override;
  Result<Instruction> Decode(const RawInstruction& raw) const override;
  std::string Format(const Instruction& instruction) const override;
};
}  // namespace c166
