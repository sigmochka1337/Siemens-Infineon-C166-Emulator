#pragma once

#include <cstdint>

namespace c166 {

struct MemoryFault {
  std::uint32_t address = 0;
};

// Fetch и Execute зависят только от этого контракта.
// Реализация Memory добавляет проверку диапазона, DPP и MMIO.
class IMemory {
 public:
  virtual ~IMemory() = default;
  virtual std::uint8_t Read8(std::uint32_t address) const = 0;
  virtual std::uint16_t Read16(std::uint32_t address) const = 0;
  virtual void Write8(std::uint32_t address, std::uint8_t value) = 0;
  virtual void Write16(std::uint32_t address, std::uint16_t value) = 0;
};

}  // namespace c166
