#pragma once

#include "interfaces/memory.h"
#include "operands.h"
#include "types.h"

namespace c166 {
class JumpUnit {
 public:
  JumpUnit(IMachine& machine) : machine_(machine) {}

  bool Jmpa(ConditionCode condition, const Operand* address);
  bool Jmpi(ConditionCode condition, const Operand* address);
  bool Jmpr(ConditionCode condition, const Operand* displacement);
  bool Jmps(const Operand* address);

 private:
  bool Jmpa(ConditionCode condition, const CodeAddressOperand* address);
  bool Jmpi(ConditionCode condition, const IndirectMemoryOperand* address);
  bool Jmpr(ConditionCode condition, const RelativeOperand* displacement);
  bool Jmps(const SegmentedAddressOperand* address);

  constexpr bool IsWordGpr(RegisterId id) noexcept {
    return id >= RegisterId::kR0 && id <= RegisterId::kR15;
  }

  IMachine& machine_;
};
}  // namespace c166
