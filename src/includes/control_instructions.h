#pragma once

#include <cstdint>

#include "src/includes/cpu_state.h"

namespace c166 {

// Контракт для инструкций без операндов и префиксов адресации.
class IControlInstructions {
 public:
  virtual ~IControlInstructions() = default;
  virtual void Nop(CpuState& state) = 0;
  virtual void ServiceWatchdog(CpuState& state) = 0;
  virtual void ExtendSegment(CpuState& state, std::uint8_t segment) = 0;
  virtual void ExtendPage(CpuState& state, std::uint8_t page) = 0;
  virtual void ExtendSegmentRegister(CpuState& state,
                                     std::uint8_t register_index) = 0;
  virtual void ExtendPageRegister(CpuState& state,
                                  std::uint8_t register_index) = 0;
};

}  // namespace c166
