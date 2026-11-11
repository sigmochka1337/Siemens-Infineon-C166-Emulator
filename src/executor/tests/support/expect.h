#pragma once

#include <cstdlib>
#include <iostream>
#include <string_view>

namespace c166::test {
inline void Expect(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(EXIT_FAILURE);
  }
}
}  // namespace c166::test
