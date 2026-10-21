#pragma once

#include <array>
#include <cstdint>

namespace c166 {

struct Psw {
  bool negative = false;
  bool carry = false;
  bool overflow = false;
  bool zero = false;
  bool user0 = false;
};

// Единственный изменяемый объект состояния процессора.
// Executor читает и изменяет его, Debugger получает из него снимки.
struct CpuState {
  std::array<std::uint16_t, 16> registers{};
  std::array<std::uint16_t, 4> dpp{};
  std::uint16_t csp = 0;
  std::uint16_t ip = 0;
  std::uint16_t sp = 0;
  std::uint16_t context = 0;
  std::uint16_t mdh = 0;
  std::uint16_t mdl = 0;
  Psw psw{};
  bool halted = false;

  std::uint32_t PhysicalPc() const {
    return (static_cast<std::uint32_t>(csp) << 16) | ip;
  }
};

}  // namespace c166
