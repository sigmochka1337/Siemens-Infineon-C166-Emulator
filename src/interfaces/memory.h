#pragma once

#include "interfaces/cpu_state.h"
#include "short_register.h"

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

  CpuState& Cpu() { return cpu_; }
  const CpuState& Cpu() const { return cpu_; }

  PhysicalAddress ResolveOperand(LogicalAddress address) {
    const auto& sequence = cpu_.sequence;
    if (sequence.prefix == Mnemonic::kExts || sequence.prefix == Mnemonic::kExtsr)
      return (PhysicalAddress(sequence.address) << 16) | address;
    if (sequence.prefix == Mnemonic::kExtp || sequence.prefix == Mnemonic::kExtpr)
      return (PhysicalAddress(sequence.address) << 14) | (address & 0x3FFF);
    if (const auto page = cpu_.previous_pages[address >> 14])
      return (PhysicalAddress(*page & 0x3FF) << 14) | (address & 0x3FFF);
    return Resolve(address);
  }

  PhysicalAddress ShortSfrAddress(RegisterId id) const {
    return (cpu_.sequence.ExtendedRegisters() ? 0xF000u : 0xFE00u) + 2 * ShortAddress(id);
  }
  Word ReadShortRegister(RegisterId id) const {
    if (IsShortSfr(id) && (cpu_.sequence.ExtendedRegisters() || int(id) >= kShortRegisterBase))
      return Read16(ShortSfrAddress(id));
    return ReadRegister(id);
  }
  void WriteShortRegister(RegisterId id, Word value) {
    if (IsShortSfr(id) && (cpu_.sequence.ExtendedRegisters() || int(id) >= kShortRegisterBase)) {
      Write16(ShortSfrAddress(id), value);
    } else {
      WriteRegister(id, value);
    }
  }

  virtual void ServiceWatchdog() { Write16(0xFEAE, Read16(0xFFAE) & 0xFF00); }
  virtual void Reset() = 0;
  virtual PhysicalAddress Resolve(LogicalAddress address) = 0;

 private:
  CpuState cpu_;
};
}  // namespace c166
