#pragma once
#include "types.h"
namespace c166 {
class IRawMemmory {
 public:
  virtual ~IRawMemmory() = default;

  virtual Byte Read8() const = 0;
  virtual void Write8() = 0;
};
}  // namespace c166
