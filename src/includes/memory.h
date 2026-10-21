#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "src/includes/memory_iface.h"

namespace c166 {

// Минимальная RAM для стенда. MMIO и DPP подключаются через тот же интерфейс.
class FlatMemory final : public IMemory {
 public:
  explicit FlatMemory(std::size_t size = 1u << 24) : bytes_(size, 0) {}

  std::uint8_t Read8(std::uint32_t address) const override;
  std::uint16_t Read16(std::uint32_t address) const override;
  void Write8(std::uint32_t address, std::uint8_t value) override;
  void Write16(std::uint32_t address, std::uint16_t value) override;

 private:
  std::vector<std::uint8_t> bytes_;
};

}  // namespace c166
