#pragma once

#include <array>
#include <string>

#include "common/instruction.h"
#include "common/result.h"

namespace c166 {
class IDecoder {
 public:
  // Uses the same encoding table as Decode; independent of supported semantics.
  virtual Result<Byte> InstructionLength(const std::array<Byte, 2>& first_word) const = 0;
  virtual Result<Instruction> Decode(const RawInstruction& raw) const = 0;
  virtual std::string Format(const Instruction& instruction) const = 0;
  virtual ~IDecoder() = default;
};
}  // namespace c166
