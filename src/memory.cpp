#include "src/includes/memory.h"

#include <stdexcept>

namespace c166 {

std::uint8_t FlatMemory::Read8(std::uint32_t address) const {
  if (address >= bytes_.size()) throw std::out_of_range("memory read8");
  return bytes_[address];
}

std::uint16_t FlatMemory::Read16(std::uint32_t address) const {
  return static_cast<std::uint16_t>(Read8(address) | (Read8(address + 1) << 8));
}

void FlatMemory::Write8(std::uint32_t address, std::uint8_t value) {
  if (address >= bytes_.size()) throw std::out_of_range("memory write8");
  bytes_[address] = value;
}

void FlatMemory::Write16(std::uint32_t address, std::uint16_t value) {
  Write8(address, static_cast<std::uint8_t>(value));
  Write8(address + 1, static_cast<std::uint8_t>(value >> 8));
}

}  // namespace c166
