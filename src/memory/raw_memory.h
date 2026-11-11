#pragma once
#include <vector>

#include "interfaces/raw_memory.h"

namespace c166 {
class RawMemory : public IRawMemory {
 public:
  RawMemory();
  Byte Read8(PhysicalAddress address) const override;
  void Write8(PhysicalAddress address, Byte value) override;

 private:
  static void CheckAddress(PhysicalAddress address);
  std::vector<Byte> data_;
};
}  // namespace c166