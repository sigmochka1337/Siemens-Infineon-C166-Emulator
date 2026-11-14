#include <limits>

#include "checks.h"
#include "exceptions.h"
#include "memory/raw_memory.h"

int main() {
  using namespace c166;

  test::Checks checks;
  RawMemory raw_memory;
  IRawMemory& raw = raw_memory;

  // The physical address space is 24 bits (C166S V1 User's Manual, chapter 4).
  constexpr PhysicalAddress kLastAddress = 0xFFFFFF;
  raw.Write8(0, 0x12);
  raw.Write8(kLastAddress, 0x34);
  checks.Equal("read first byte", raw.Read8(0), 0x12);
  checks.Equal("read last byte", raw.Read8(kLastAddress), 0x34);

  for (const PhysicalAddress address :
       {kLastAddress + 1, std::numeric_limits<PhysicalAddress>::max()}) {
    checks.Throws<PhysicalAddressOutOfRange>("raw read outside memory", address,
                                             [&] { (void)raw.Read8(address); });
    checks.Throws<PhysicalAddressOutOfRange>("raw write outside memory", address,
                                             [&] { raw.Write8(address, 0x56); });
  }

  checks.Equal("first byte after rejected writes", raw.Read8(0), 0x12);
  checks.Equal("last byte after rejected writes", raw.Read8(kLastAddress), 0x34);
  return checks.Result();
}
