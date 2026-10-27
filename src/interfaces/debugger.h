#pragma once
#include <string>

#include "types.h"

namespace c166 {
class IDebugger {
 public:
  virtual ~IDebugger() = default;

  virtual void Step() = 0;
  virtual void Run() = 0;
  virtual void AddBreakpoint(PhysicalAddress address) = 0;
  virtual void RemoveBreakpoint(PhysicalAddress address) = 0;

  virtual Snapshot Trace() const = 0;
  virtual Byte PhysicalRead8(PhysicalAddress address) const = 0;
  virtual Word PhysicalRead16(PhysicalAddress address) const = 0;
  virtual void PhysicalWrite8(PhysicalAddress address, Byte value) = 0;
  virtual void PhysicalWrite16(PhysicalAddress address, Word value) = 0;

  virtual Byte LogicalRead8(LogicalAddress address) const = 0;
  virtual Word LogicalRead16(LogicalAddress address) const = 0;
  virtual void LogicalWrite8(LogicalAddress address, Byte value) = 0;
  virtual void LogicalWrite16(LogicalAddress address, Word value) = 0;

  virtual std::string NextInstructionToString() const = 0;
};
}  // namespace c166
