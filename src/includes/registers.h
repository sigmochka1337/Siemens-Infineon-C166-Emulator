#pragma once

#include "src/includes/cpu_state.h"

namespace c166 {

class IRegisters {
 public:
  virtual ~IRegisters() = default;
  virtual std::uint16_t Read(const CpuState& state,
                             std::uint8_t index) const = 0;
  virtual void Write(CpuState& state, std::uint8_t index,
                     std::uint16_t value) = 0;
};

}  // namespace c166
