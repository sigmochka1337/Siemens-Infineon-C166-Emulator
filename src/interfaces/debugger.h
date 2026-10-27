#pragma once
#include "types.h"

namespace c166 {
class IDebugger {
 public:
  virtual ~IDebugger() = default;

  virtual void Step() = 0;
  virtual void Run() = 0;
  virtual void AddBreakpoint(PhysicalAddress address) = 0;
  virtual void RemoveBreakpoint(PhysicalAddress address) = 0;

  virtual Snapshot Trace() = 0;
  virtual Byte Read8(PhysicalAddress address) const = 0;
  virtual Word Read16(PhysicalAddress address) const = 0;
  virtual void Write8(PhysicalAddress address, Byte value) = 0;
  virtual void Write16(PhysicalAddress address, Word value) = 0;

  virtual std::string NextInstructionToString() const = 0;
};
}  // namespace c166
