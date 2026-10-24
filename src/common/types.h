#pragma once

#include <cstdint>

namespace c166 {
using Byte = uint8_t;
using Word = uint16_t;
using Dword = uint32_t;
using PhysicalAddress = uint32_t;
inline constexpr PhysicalAddress kMaxPhysicalAddress = 0xFFFFFF;

struct LogicalAddress {
  Word value = 0;
};
struct CodeAddress {
  Byte segment = 0;
  Word offset = 0;
  bool operator==(const CodeAddress&) const = default;
};
constexpr PhysicalAddress ToPhysicalAddress(CodeAddress address) {
  return (static_cast<PhysicalAddress>(address.segment) << 16) | address.offset;
}
enum class Width : uint8_t { kByte = 1, kWord = 2 };

// Encoded Rb order is RL0, RH0, RL1, RH1, ... RL7, RH7.
enum class RegisterId : uint8_t {
  kR0,
  kR1,
  kR2,
  kR3,
  kR4,
  kR5,
  kR6,
  kR7,
  kR8,
  kR9,
  kR10,
  kR11,
  kR12,
  kR13,
  kR14,
  kR15,
  kRL0,
  kRH0,
  kRL1,
  kRH1,
  kRL2,
  kRH2,
  kRL3,
  kRH3,
  kRL4,
  kRH4,
  kRL5,
  kRH5,
  kRL6,
  kRH6,
  kRL7,
  kRH7,
  kSP,
  kCP,
  kPSW,
  kDPP0,
  kDPP1,
  kDPP2,
  kDPP3,
  kMDL,
  kMDH,
  kMDC,
  kSTKOV,
  kSTKUN,
};
namespace psw {
inline constexpr Word kN = 1u << 0;
inline constexpr Word kC = 1u << 1;
inline constexpr Word kV = 1u << 2;
inline constexpr Word kZ = 1u << 3;
inline constexpr Word kE = 1u << 4;
inline constexpr Word kMulip = 1u << 5;
inline constexpr Word kUsr0 = 1u << 6;
inline constexpr Word kConditionMask = kN | kC | kV | kZ | kE;
}  // namespace psw

enum class ExtensionMode : uint8_t { kNone, kPage, kSegment };
struct ExtensionState {
  ExtensionMode mode = ExtensionMode::kNone;
  Word value = 0;
  Byte remaining = 0;  // 1..4 subsequent executed instructions; zero means inactive.
  bool extended_sfr = false;
};
struct BitAddress {
  Byte offset = 0;  // Encoded bitoff, not a physical byte address.
  Byte bit = 0;
};
struct BitLocation {
  PhysicalAddress word_address = 0;
  Byte bit = 0;
};
}  // namespace c166
