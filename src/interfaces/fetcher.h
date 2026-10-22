#pragma once
#include "interfaces/memory.h"
namespace c166 {

enum class FetchError : uint8_t {
  kNone,
  kUnmapped,
  kNotExecutable,
  kMisaligned,
  kTruncated,
};

struct RawInstruction {
  Byte bytes[8]{};
  uint8_t size = 0;
  CodeAddress pc{};
  FetchError error = FetchError::kNone;
};

class IFetcher {
 public:
  virtual RawInstruction Fetch(CodeAddress pc, const IMemoryBus& mem) const = 0;

  virtual ~IFetcher() = default;
};
}  // namespace c166
