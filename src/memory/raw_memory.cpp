#include "memory/raw_memory.h"

#include "exceptions.h"

namespace c166 {
RawMemory::RawMemory() : data_(kMaxPhysicalAddress + 1, 0) {}
Byte RawMemory::Read8(PhysicalAddress address) const {
  CheckAddress(address);
  return data_[address];
}
void RawMemory::Write8(PhysicalAddress address, Byte value) {
  CheckAddress(address);
  data_[address] = value;
}
void RawMemory::CheckAddress(PhysicalAddress address) {
  if (address > kMaxPhysicalAddress) {
    throw PhysicalAddressOutOfRange(address);
  }
}
}  // namespace c166