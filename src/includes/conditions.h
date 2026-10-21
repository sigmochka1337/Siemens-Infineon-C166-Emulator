#pragma once

#include "src/includes/cpu_state.h"

namespace c166 {

enum class Condition : std::uint8_t {
  kAlways,
  kZero,
  kNotZero,
  kCarry,
  kNotCarry,
  kNegative,
  kNotNegative
};
bool ConditionHolds(Condition condition, const Psw& psw);

}  // namespace c166
