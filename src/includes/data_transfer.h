#pragma once

#include <cstdint>

#include "src/includes/cpu_state.h"
#include "src/includes/memory_iface.h"

namespace c166 {

// Контракт для MOV/MOVB и префиксов адресной области.
class IDataTransfer {
 public:
  virtual ~IDataTransfer() = default;
  virtual void MoveRegister(CpuState& state, std::uint8_t destination,
                            std::uint8_t source) = 0;
  virtual void MoveRegisterImmediate(CpuState& state, std::uint8_t destination,
                                     std::uint16_t value) = 0;
  virtual void MoveRegisterMemory(CpuState& state, IMemory& memory,
                                  std::uint8_t destination,
                                  std::uint32_t address) = 0;
  virtual void MoveMemoryRegister(CpuState& state, IMemory& memory,
                                  std::uint32_t address,
                                  std::uint8_t source) = 0;
  virtual void MoveByteRegister(CpuState& state, std::uint8_t destination,
                                std::uint8_t source) = 0;
  virtual void MoveByteImmediate(CpuState& state, std::uint8_t destination,
                                 std::uint8_t value) = 0;
  virtual void MoveByteMemory(CpuState& state, IMemory& memory,
                              std::uint8_t destination,
                              std::uint32_t address) = 0;
  virtual void MoveMemoryByte(CpuState& state, IMemory& memory,
                              std::uint32_t address, std::uint8_t source) = 0;
};

}  // namespace c166
