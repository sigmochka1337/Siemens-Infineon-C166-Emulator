#pragma once

#include "types.h"

namespace c166 {
class IMachine {
 public:
  virtual ~IMachine() = default;

  virtual Byte Read8(PhysicalAddress address) const = 0;
  virtual void Write8(PhysicalAddress address, Byte value) = 0;
  virtual Word Read16(PhysicalAddress address) const = 0;
  virtual void Write16(PhysicalAddress address, Word value) = 0;

  virtual Word ReadRegister(RegisterId id) const = 0;
  virtual void WriteRegister(RegisterId id, Word value) = 0;
  virtual CodeAddress GetPC() const = 0;
  virtual void SetPC(CodeAddress pc) = 0;

  virtual void Reset() = 0;
  virtual PhysicalAddress Resolve(LogicalAddress address) = 0;
};
}  // namespace c166
