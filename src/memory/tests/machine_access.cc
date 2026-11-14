#include <array>

#include "checks.h"
#include "exceptions.h"
#include "memory/machine.h"
#include "memory/raw_memory.h"

namespace {

using namespace c166;

void CheckPhysicalRange(test::Checks& checks, IMachine& machine) {
  constexpr PhysicalAddress kOutside = 0x1000000;

  checks.Throws<PhysicalAddressOutOfRange>("byte read outside memory", kOutside,
                                           [&] { (void)machine.Read8(kOutside); });
  checks.Throws<PhysicalAddressOutOfRange>("byte write outside memory", kOutside,
                                           [&] { machine.Write8(kOutside, 0); });
  checks.Throws<PhysicalAddressOutOfRange>("word read outside memory", kOutside,
                                           [&] { (void)machine.Read16(kOutside); });
  checks.Throws<PhysicalAddressOutOfRange>("word write outside memory", kOutside,
                                           [&] { machine.Write16(kOutside, 0); });
}

void CheckAlignment(test::Checks& checks, IMachine& machine, IRawMemory& raw) {
  // Bytes can be addressed individually. Odd word accesses trigger ILLOPA (pp. 3-31, 3-54).
  constexpr PhysicalAddress kEven = 0xFA80;
  constexpr PhysicalAddress kOdd = kEven + 1;
  machine.Write16(kEven, 0x1234);
  machine.Write8(kOdd, 0x56);
  checks.Equal("odd byte read", machine.Read8(kOdd), 0x56);
  checks.Equal("aligned word read", machine.Read16(kEven), 0x5634);
  raw.Write8(kOdd + 1, 0x78);

  checks.Throws<IllegalWordOperandAccess>("odd word read", kOdd,
                                          [&] { (void)machine.Read16(kOdd); });
  checks.Throws<IllegalWordOperandAccess>("odd word write", kOdd,
                                          [&] { machine.Write16(kOdd, 0xABCD); });

  checks.Equal("low byte of rejected word write", raw.Read8(kOdd), 0x56);
  checks.Equal("high byte of rejected word write", raw.Read8(kOdd + 1), 0x78);
}

void CheckFlatMemory(test::Checks& checks, Machine& machine) {
  constexpr PhysicalAddress kLastByte = 0xFFFFFF;
  machine.Write16(kLastByte - 1, 0xABCD);
  checks.Equal("last aligned word", machine.Read16(kLastByte - 1), 0xABCD);
  checks.Equal("last byte", machine.Read8(kLastByte), 0xAB);
  checks.Throws<IllegalWordOperandAccess>("word write at last byte", kLastByte,
                                          [&] { machine.Write16(kLastByte, 0); });
  checks.Equal("last byte after rejected write", machine.Read8(kLastByte), 0xAB);

  constexpr std::array<Byte, 2> kImage{0x34, 0x12};
  machine.LoadBinary(0x100000, kImage);
  machine.WriteRegister(RegisterId::kDPP1, 0x40);
  const PhysicalAddress address = machine.Resolve(0x4000);
  checks.Equal("DPP1 physical address", address, 0x100000);
  checks.Equal("loaded word", machine.Read16(address), 0x1234);
}

}  // namespace

int main() {
  c166::test::Checks checks;
  c166::RawMemory raw;
  c166::Machine machine(raw);
  CheckPhysicalRange(checks, machine);
  CheckAlignment(checks, machine, raw);
  CheckFlatMemory(checks, machine);
  return checks.Result();
}
