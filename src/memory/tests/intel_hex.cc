#include <stdexcept>
#include <string>
#include <string_view>

#include "checks.h"
#include "memory/machine.h"
#include "memory/raw_memory.h"

namespace {

using namespace c166;

void CheckLinearImage(test::Checks& checks, Machine& machine) {
  machine.SetPC({1, 0x0200});
  machine.Write8(0x120012, 0xCC);
  machine.LoadIntelHex(
      "\xEF\xBB\xBF:020000040012E8\r\n"
      ":01001400ba31\r\n"
      " :02001000A55AEF \r\n"
      ":0400000300100002E7\r\n"
      ":0400000500120010D5\r\n"
      ":0200000400FEFC\r\n"
      ":02FFFF001234BA\r\n"
      ":0200000400FFFB\r\n"
      ":01FFFF00AC55\r\n"
      ":00000001FF");

  checks.Equal("linear address", machine.Read16(0x120010), 0x5AA5);
  checks.Equal("gap", machine.Read8(0x120012), 0xCC);
  checks.Equal("out-of-order record", machine.Read8(0x120014), 0xBA);
  checks.Equal("end of linear page", machine.Read8(0xFEFFFF), 0x12);
  checks.Equal("beginning of next linear page", machine.Read8(0xFF0000), 0x34);
  checks.Equal("last physical byte", machine.Read8(0xFFFFFF), 0xAC);
  checks.Equal("PC after loading", machine.GetPC().ToPhysicalAddress(), 0x010200);
}

void CheckSegmentedImage(test::Checks& checks, Machine& machine) {
  machine.LoadIntelHex(
      ":020000021234B6\n"
      ":02FFFF00DEAD75\n"
      ":00000001FF\n\n");

  checks.Equal("last byte of segment", machine.Read8(0x2233F), 0xDE);
  checks.Equal("wrapped segment offset", machine.Read8(0x12340), 0xAD);
}

void CheckMalformedImages(test::Checks& checks, Machine& machine) {
  constexpr std::string_view kValidData = ":01010000AA54\n";
  constexpr std::string_view malformed_tails[] = {
      "",
      ":01010200BB40\n:00000001FF\n",  // Bad checksum.
      ":02010200BB40\n:00000001FF\n",  // Byte count does not match.
      ":01010200BZ41\n:00000001FF\n",
      ":00000006FA\n:00000001FF\n",      // Unknown record type.
      ":020001020000FB\n:00000001FF\n",  // Nonzero offset in type 02.
      ":0100000400FB\n:00000001FF\n",    // Wrong size of type 04.
      ":020000050000F9\n:00000001FF\n",  // Wrong size of type 05.
      ":0100000101FD\n",                 // EOF with data.
      ":00000001FF\n:01010200BB41\n",
  };

  machine.Write8(0x0100, 0xCC);
  for (const auto tail : malformed_tails) {
    std::string image(kValidData);
    image += tail;
    bool rejected = false;
    try {
      machine.LoadIntelHex(image);
    } catch (const std::invalid_argument&) {
      rejected = true;
    }
    checks.Equal(image, rejected, true);
    checks.Equal("malformed image preserves memory", machine.Read8(0x0100), 0xCC);
  }
}

void CheckOutOfRange(test::Checks& checks, Machine& machine) {
  machine.Write8(0xFFFFFF, 0xCC);
  bool rejected = false;
  try {
    machine.LoadIntelHex(
        ":0200000400FFFB\n"
        ":02FFFF001234BA\n"
        ":00000001FF\n");
  } catch (const std::out_of_range&) {
    rejected = true;
  }
  checks.Equal("record crossing the physical limit", rejected, true);
  checks.Equal("out-of-range image preserves memory", machine.Read8(0xFFFFFF), 0xCC);
}

}  // namespace

int main() {
  c166::test::Checks checks;
  c166::RawMemory raw;
  c166::Machine machine(raw);
  CheckLinearImage(checks, machine);
  CheckSegmentedImage(checks, machine);
  CheckMalformedImages(checks, machine);
  CheckOutOfRange(checks, machine);
  return checks.Result();
}
