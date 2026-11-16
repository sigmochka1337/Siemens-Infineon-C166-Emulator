#pragma once

#include <cstdlib>
#include <format>
#include <iostream>
#include <string_view>

#include "types.h"

namespace c166::test {

class Checks {
 public:
  void BeginCase(std::string_view name) {
    case_name_ = name;
    ++cases_;
    case_failed_ = false;
  }

  void Equal(std::string_view field, Dword actual, Dword expected) {
    if (actual != expected) {
      Fail(std::format("{} expected 0x{:X}, got 0x{:X}", field, expected, actual));
    }
  }

  void Fail(std::string_view message) {
    if (!case_failed_) {
      ++failed_cases_;
      case_failed_ = true;
    }
    std::cerr << std::format("{}: {}\n", case_name_, message);
  }

  int Result() const {
    std::cout << cases_ - failed_cases_ << " cases passed, " << failed_cases_ << " failed\n";
    return failed_cases_ == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
  }

 private:
  std::string_view case_name_;
  int cases_ = 0;
  int failed_cases_ = 0;
  bool case_failed_ = false;
};

}  // namespace c166::test
