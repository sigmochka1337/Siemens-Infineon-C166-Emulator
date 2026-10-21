#pragma once

#include "src/includes/conditions.h"
#include "src/includes/cpu_state.h"
#include "src/includes/memory_iface.h"

namespace c166 {

class IStack {
 public:
  virtual ~IStack() = default;
  virtual void Push(CpuState& state, IMemory& memory, std::uint16_t value) = 0;
  virtual std::uint16_t Pop(CpuState& state, IMemory& memory) = 0;
  virtual void Call(CpuState& state, IMemory& memory, std::uint32_t target) = 0;
  virtual void CallA(CpuState& state, IMemory& memory, Condition condition,
                     std::uint32_t target) = 0;
  virtual void CallI(CpuState& state, IMemory& memory, Condition condition,
                     std::uint8_t register_index) = 0;
  virtual void CallR(CpuState& state, IMemory& memory,
                     std::int16_t displacement) = 0;
  virtual void CallS(CpuState& state, IMemory& memory, std::uint8_t segment,
                     std::uint16_t offset) = 0;
  virtual void PCall(CpuState& state, IMemory& memory, std::uint16_t value,
                     std::uint32_t target) = 0;
  virtual void Return(CpuState& state, IMemory& memory) = 0;
  virtual void RetS(CpuState& state, IMemory& memory) = 0;
  virtual void RetP(CpuState& state, IMemory& memory, std::uint16_t value) = 0;
  virtual void JumpA(CpuState& state, Condition condition,
                     std::uint32_t target) = 0;
  virtual void JumpI(CpuState& state, Condition condition,
                     std::uint8_t register_index) = 0;
  virtual void JumpR(CpuState& state, std::int16_t displacement) = 0;
  virtual void JumpS(CpuState& state, std::uint8_t segment,
                     std::uint16_t offset) = 0;
  virtual void JumpBit(CpuState& state, IMemory& memory, std::uint32_t bit,
                       std::int16_t displacement, Condition condition) = 0;
};

}  // namespace c166
