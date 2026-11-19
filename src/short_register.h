#pragma once

#include <array>

#include "types.h"

namespace c166 {
// Eight-bit "reg" addresses below F0 select an SFR (V1, table 3-12).
// Keep named core registers available to the debugger and preserve other short addresses.
inline constexpr int kShortRegisterBase = 0x100;
inline constexpr std::array<Byte, 14> kCoreShortAddresses{0x09, 0x08, 0x88, 0x00, 0x01, 0x02, 0x03,
                                                          0x07, 0x06, 0x87, 0x0A, 0x0B, 0x89, 0xD6};
static_assert(int(RegisterId::kTFR) - int(RegisterId::kSP) + 1 == kCoreShortAddresses.size());

constexpr bool IsShortSfr(RegisterId id) {
  return (id >= RegisterId::kSP && id <= RegisterId::kTFR) ||
         (int(id) >= kShortRegisterBase && int(id) < kShortRegisterBase + 0xF0);
}
constexpr Byte ShortAddress(RegisterId id) {
  if (int(id) >= kShortRegisterBase) return static_cast<Byte>(int(id) - kShortRegisterBase);
  return kCoreShortAddresses[int(id) - int(RegisterId::kSP)];
}
constexpr RegisterId DecodeShortRegister(Byte address, bool byte) {
  if (address >= 0xF0) {
    return static_cast<RegisterId>(int(byte ? RegisterId::kRL0 : RegisterId::kR0) + (address & 15));
  }
  for (size_t i = 0; i < kCoreShortAddresses.size(); ++i) {
    if (kCoreShortAddresses[i] == address) return static_cast<RegisterId>(int(RegisterId::kSP) + i);
  }
  return static_cast<RegisterId>(kShortRegisterBase + address);
}
}  // namespace c166
