#pragma once

#include <span>

#include "common/result.h"
#include "common/state.h"

namespace c166 {
enum class AccessKind : uint8_t { kCode, kData, kStack };
class IMemoryBus {
 public:
  // CPU reads may have MMIO side effects. Addresses here are physical.
  virtual Result<Byte> Read8(PhysicalAddress address, AccessKind kind) = 0;
  virtual Result<Word> Read16(PhysicalAddress address, AccessKind kind) = 0;
  virtual Result<void> Write8(PhysicalAddress address, Byte value, AccessKind kind) = 0;
  virtual Result<void> Write16(PhysicalAddress address, Word value, AccessKind kind) = 0;
  // Inspection must not consume input, clear device flags, or change CPU state.
  virtual Result<Byte> Peek8(PhysicalAddress address) const = 0;
  virtual ~IMemoryBus() = default;
};
class IRegisterBus {
 public:
  // Guest reads may affect control registers (e.g. MDL/MDC); use Snapshot to inspect.
  virtual Result<Word> ReadRegister(RegisterId id) = 0;
  virtual Result<void> WriteRegister(RegisterId id, Word value) = 0;
  virtual CodeAddress GetPC() const = 0;
  // Privileged core/debugger operation, not a guest memory-mapped CSP/IP write.
  virtual Result<void> SetPC(CodeAddress pc) = 0;
  virtual ExtensionState GetExtension() const = 0;
  virtual void SetExtension(ExtensionState state) = 0;
  virtual ~IRegisterBus() = default;
};
// One implementation backs both bases: GPR/RAM and SFR/register aliases agree.
// The core cannot be constructed with unrelated register and memory stores.
class IMachine : public IMemoryBus, public IRegisterBus {
 public:
  virtual void Reset() = 0;
  virtual CpuSnapshot Snapshot() const = 0;
  // Loader validates the whole range before writing; no device side effects.
  virtual Result<void> LoadBinary(PhysicalAddress address, std::span<const Byte> bytes) = 0;
  virtual Result<PhysicalAddress> ResolveDataAddress(LogicalAddress address) const = 0;
  virtual Result<PhysicalAddress> ResolveShortRegister(Byte offset, Width width) const = 0;
  virtual Result<BitLocation> ResolveBitAddress(BitAddress address) const = 0;
  ~IMachine() override = default;
};
}  // namespace c166
