#pragma once

#include <cstdlib>
#include <exception>
#include <format>
#include <iostream>
#include <string_view>

#include "types.h"

namespace c166::test {

class Checks {
 public:
  void Equal(std::string_view name, Dword actual, Dword expected) {
    if (actual != expected) {
      Fail(name, std::format("expected 0x{:X}, got 0x{:X}", expected, actual));
    }
  }

  template <class Exception>
  void Throws(std::string_view name, PhysicalAddress address, const auto& access) {
    try {
      access();
      Fail(name, "expected a memory exception");
    } catch (const Exception& error) {
      Equal(name, error.GetAddress(), address);
    } catch (const std::exception& error) {
      Fail(name, std::format("unexpected exception type ({})", error.what()));
    }
  }

  int Result() const { return failed_ ? EXIT_FAILURE : EXIT_SUCCESS; }

 private:
  void Fail(std::string_view name, std::string_view message) {
    std::cerr << name << ": " << message << '\n';
    failed_ = true;
  }

  bool failed_ = false;
};

}  // namespace c166::test
