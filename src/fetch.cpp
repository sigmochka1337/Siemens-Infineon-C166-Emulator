#include <stdexcept>

#include "src/includes/decoder.h"

namespace c166 {

Fetched Fetch(const IMemory& memory, std::uint32_t address) {
  Fetched result;
  result.pc_start = address;
  try {
    for (; result.valid < kMaxInstructionBytes; ++result.valid)
      result.bytes[result.valid] = memory.Read8(address + result.valid);
  } catch (const std::out_of_range&) {
    result.fault = result.valid == 0;
  }
  return result;
}

}  // namespace c166
