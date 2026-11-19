#pragma once

#include <optional>

#include "interfaces/memory.h"
#include "operands.h"

namespace c166 {

// Snapshot before updating PSW. An explicit bit write to PSW uses the original word (p. 5-25).
struct AddressedBit {
  PhysicalAddress address;
  Word word;
  Word mask;

  bool IsSet() const { return (word & mask) != 0; }

  void Write(IMachine& machine, bool value) const {
    machine.Write16(address, value ? word | mask : word & ~mask);
  }

  void UpdateFlags(IMachine& machine) const {
    const Word psw = machine.ReadRegister(RegisterId::kPSW) & ~psw::kConditionMask;
    machine.WriteRegister(RegisterId::kPSW, psw | (IsSet() ? psw::kN : psw::kZ));
  }
};

inline std::optional<PhysicalAddress> ResolveBitWordAddress(const IMachine& machine,
                                                            Word word_address) {
  if ((word_address & 1) != 0) return std::nullopt;

  // V1 table 3-12. The operand carries the decoded word address; bitoff F0..FF
  // appears as FFE0..FFFE and always selects R0..R15 in the current context.
  PhysicalAddress address = word_address;
  if (address >= 0xFFE0) {
    address = machine.ReadRegister(RegisterId::kCP) + (address - 0xFFE0);
  } else {
    const bool ram = address >= 0xFD00 && address <= 0xFDFE;
    const bool sfr = address >= 0xFF00 && address <= 0xFFDE;
    const bool esfr = address >= 0xF100 && address <= 0xF1DE;
    if (sfr && machine.Cpu().sequence.ExtendedRegisters()) address -= 0xE00;
    if (!ram && !sfr && !esfr) {
      // A caller may also supply an already resolved word of the active GPR bank.
      const PhysicalAddress cp = machine.ReadRegister(RegisterId::kCP);
      if (address < cp || address > cp + 30) return std::nullopt;
    }
  }
  return address;
}

inline std::optional<AddressedBit> ReadAddressedBit(const IMachine& machine, const Operand* src) {
  const auto* bit = dynamic_cast<const BitOperand*>(src);
  if (!bit || bit->Bit() > 15) return std::nullopt;
  const auto address = ResolveBitWordAddress(machine, bit->WordAddress());
  if (!address) return std::nullopt;
  return AddressedBit{*address, machine.Read16(*address), static_cast<Word>(1u << bit->Bit())};
}

}  // namespace c166
