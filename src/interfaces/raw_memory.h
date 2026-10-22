#pragma once

#include <cstdint>
#include <optional>

namespace c166 {
using PhysicalAddress = uint32_t;
using OperandAddress = uint32_t;
using Byte = uint8_t;
using Word = uint16_t;
using Dword = uint32_t;

struct CodeAddress {
  Byte segment = 0;
  Word offset = 0;
};

enum class MemoryFaultKind : uint8_t {
  kUnmapped,
  kOutOfRange,
  kReadOnly,
  kMisaligned,
  kNotExecutable,
  kDeviceError,
};

template <typename T>
struct ReadResult {
  std::optional<T> value;
  std::optional<MemoryFaultKind> fault;

  bool Ok() const { return value.has_value() && !fault.has_value(); }
};

struct WriteResult {
  std::optional<MemoryFaultKind> fault;

  bool Ok() const { return !fault.has_value(); }
};

class IRawMemory {
 public:
  virtual WriteResult Write8(PhysicalAddress address, Byte value) = 0;
  virtual WriteResult Write16(PhysicalAddress address, Word value) = 0;
  virtual WriteResult Write32(PhysicalAddress address, Dword value) = 0;

  virtual ReadResult<Byte> Read8(PhysicalAddress address) const = 0;
  virtual ReadResult<Word> Read16(PhysicalAddress address) const = 0;
  virtual ReadResult<Dword> Read32(PhysicalAddress address) const = 0;

  virtual ~IRawMemory() = default;
};
}  // namespace c166
