#pragma once

#include "types.h"

namespace c166::test {

// C166S V1 SubSystem User's Manual v1.6, pp. 3-17, 3-23 and 3-27–3-29.
inline constexpr Word kNegative = 0x0001;
inline constexpr Word kCarry = 0x0002;
inline constexpr Word kOverflow = 0x0004;
inline constexpr Word kZero = 0x0008;
inline constexpr Word kEndOfTable = 0x0010;
inline constexpr Word kPriorityMask = 0xF000;
inline constexpr Word kIllegalWord = 0x0004;
inline constexpr Word kSegmentationDisabled = 0x0800;
inline constexpr Word kResetStackPointer = 0xFC00;  // p. 3-61
inline constexpr CodeAddress kClassBVector{0, 0x0028};

}  // namespace c166::test
