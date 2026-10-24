#pragma once

#include <array>

#include "common/types.h"

namespace c166 {
// Observation only: RAM is not included, so this is not a serialization format.
struct CpuSnapshot {
  std::array<Word, 16> registers{};
  CodeAddress pc{};
  Word sp = 0;
  Word cp = 0;
  Word psw = 0;
  std::array<Word, 4> dpp{};
  Word mdl = 0;
  Word mdh = 0;
  Word mdc = 0;
  Word stkov = 0;
  Word stkun = 0;
  ExtensionState extension{};
};
}  // namespace c166
