#include <array>
#include <iostream>
#include <stdexcept>
#include <string>

#include "memory/machine.h"

namespace {
using namespace c166;

void Require(bool condition, const char* message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

template <typename T>
T Value(Result<T> result) {
  if (!result.Ok()) {
    throw std::runtime_error(result.Error().message);
  }
  return result.Value();
}

void Check(Result<void> result) {
  if (!result.Ok()) {
    throw std::runtime_error(result.Error().message);
  }
}

void RegisterAndMemoryAliases() {
  Machine machine;
  Check(machine.WriteRegister(RegisterId::kR0, 0x12FF));
  Require(Value(machine.Read16(0xFC00, AccessKind::kData)) == 0x12FF, "R0 aliases RAM");
  Check(machine.WriteRegister(RegisterId::kRH0, 0x13));
  Require(Value(machine.ReadRegister(RegisterId::kR0)) == 0x13FF, "RH0 preserves RL0");
  Check(machine.Write8(0xFC00, 0x34, AccessKind::kData));
  Require(Value(machine.ReadRegister(RegisterId::kR0)) == 0x1334, "RAM write updates GPR");
  Check(machine.WriteRegister(RegisterId::kCP, 0xF400));
  Check(machine.WriteRegister(RegisterId::kR0, 0xABCD));
  Require(Value(machine.Read16(0xF400, AccessKind::kData)) == 0xABCD, "CP selects bank");
  Check(machine.Write16(0xFE10, 0xFC00, AccessKind::kData));
  Require(Value(machine.ReadRegister(RegisterId::kR0)) == 0x1334, "Switch back preserves bank");
  auto invalid_bank = machine.WriteRegister(RegisterId::kCP, 0xFDF0);
  Require(!invalid_bank.Ok(), "Reject CP bank crossing the DPRAM boundary");
  Require(machine.Snapshot().cp == 0xFC00, "Rejected CP write is atomic");
  Check(machine.WriteRegister(RegisterId::kCP, 1));
  Require(machine.Snapshot().cp == 0xFC00, "CP fixed bits and low-address mapping");
}

void SfrWidthAndSideEffects() {
  Machine machine;
  Check(machine.Write16(0xFE02, 0xFFFF, AccessKind::kData));
  Require(Value(machine.ReadRegister(RegisterId::kDPP1)) == 0x3FF, "DPP mask and alias");
  Check(machine.WriteRegister(RegisterId::kPSW, 0xFC5A));
  Check(machine.Write8(0xFF10, 1, AccessKind::kData));
  Require(machine.Snapshot().psw == 1, "Low SFR byte write clears high byte");
  Check(machine.Write8(0xFF11, 0x80, AccessKind::kData));
  Require(machine.Snapshot().psw == 0x8000, "High SFR byte write clears low byte");
  Check(machine.WriteRegister(RegisterId::kSP, 0x1235));
  Require(Value(machine.Read16(0xFE12, AccessKind::kData)) == 0xF234, "SP fixed bits");
  Check(machine.WriteRegister(RegisterId::kMDH, 0x1234));
  Require((machine.Snapshot().mdc & 0x10) != 0, "MDH store sets MDRIU");
  Value(machine.Peek8(0xFE0E));
  Require((machine.Snapshot().mdc & 0x10) != 0, "Debugger inspection preserves MDRIU");
  Value(machine.ReadRegister(RegisterId::kMDL));
  Require((machine.Snapshot().mdc & 0x10) == 0, "CPU MDL read clears MDRIU");
  Require(!machine.Read16(0xFE08, AccessKind::kData).Ok(), "Unimplemented SFR is explicit");
  auto code_sfr = machine.Read8(0xFF10, AccessKind::kCode);
  Require(!code_sfr.Ok() && code_sfr.Error().code == FaultCode::kNotExecutable,
          "Cannot fetch instructions from SFR");
}

void AddressResolution() {
  Machine machine;
  Check(machine.WriteRegister(RegisterId::kDPP0, 4));
  Check(machine.WriteRegister(RegisterId::kDPP1, 4));
  Require(Value(machine.ResolveDataAddress({0x0123})) == 0x10123, "DPP page translation");
  Require(Value(machine.ResolveDataAddress({0x4123})) == 0x10123, "DPP mapping is not injective");
  machine.SetExtension({ExtensionMode::kPage, 0x123, 3, false});
  Require(Value(machine.ResolveDataAddress({0xC456})) == 0x48C456, "EXTP replaces page");
  machine.SetExtension({ExtensionMode::kSegment, 0xAB, 2, false});
  Require(Value(machine.ResolveDataAddress({0xC456})) == 0xABC456, "EXTS uses full offset");
  Require(Value(machine.ResolveShortRegister(0xF1, Width::kWord)) == 0xFC02,
          "Short word GPR address uses CP");
  Require(Value(machine.ResolveShortRegister(0xF1, Width::kByte)) == 0xFC01,
          "Short byte GPR address selects RH0");
  Require(Value(machine.ResolveShortRegister(0x88, Width::kByte)) == 0xFF10,
          "Short SFR byte address points to low byte");
  Require(Value(machine.ResolveBitAddress({0, 15})).word_address == 0xFD00,
          "Bit RAM has fixed base");
  Require(Value(machine.ResolveBitAddress({0x88, 4})).word_address == 0xFF10,
          "Bit addressing of PSW");
  machine.SetExtension({ExtensionMode::kPage, 4, 1, true});
  Require(Value(machine.ResolveShortRegister(0x88, Width::kWord)) == 0xF110, "EXT R selects ESFR");
  Require(Value(machine.ResolveBitAddress({0x88, 4})).word_address == 0xF110,
          "EXT R also selects bit ESFR");
  Require(!machine.ResolveBitAddress({0x88, 16}).Ok(), "Reject invalid bit index");
  machine.SetExtension({ExtensionMode::kPage, 0x123, 0, true});
  Require(Value(machine.ResolveDataAddress({0x0123})) == 0x10123, "Expired EXT uses DPP");
}

void FaultsLoaderAndReset() {
  Machine machine;
  auto odd = machine.Read16(0x1001, AccessKind::kData);
  Require(!odd.Ok() && odd.Error().code == FaultCode::kTrap &&
              odd.Error().trap == TrapKind::kIllegalWordAccess,
          "Odd CPU word access is an architectural trap");
  Require(!machine.Read8(0x1000000, AccessKind::kData).Ok(), "24-bit bounds");
  const auto outside_word = machine.Read16(0x1000001, AccessKind::kData);
  Require(!outside_word.Ok() && outside_word.Error().code == FaultCode::kOutOfRange &&
              !IsTrap(outside_word.Error()),
          "Invalid host physical address is not an architectural alignment trap");
  Check(machine.Write8(0xFFFFFF, 0xAA, AccessKind::kData));
  Require(Value(machine.Peek8(0xFFFFFF)) == 0xAA, "Highest physical byte is accessible");
  Check(machine.Write8(0xEFFE, 0x55, AccessKind::kData));
  const std::array<Byte, 4> crosses_sfr{1, 2, 3, 4};
  Require(!machine.LoadBinary(0xEFFE, crosses_sfr).Ok(), "Loader rejects SFR crossing");
  Require(Value(machine.Peek8(0xEFFE)) == 0x55, "Failed load does not partially write");
  const std::array<Byte, 3> crosses_end{1, 2, 3};
  Require(!machine.LoadBinary(0xFFFFFE, crosses_end).Ok(), "Loader checks complete range");
  Require(Value(machine.Peek8(0xFFFFFF)) == 0xAA, "Out-of-range load is atomic");
  const std::array<Byte, 2> program{0xCC, 0};
  Check(machine.LoadBinary(0x1000, program));
  Check(machine.SetPC({3, 0x1000}));
  auto bad_pc = machine.SetPC({3, 0x1001});
  Require(!bad_pc.Ok() && bad_pc.Error().code == FaultCode::kMisaligned && !bad_pc.Error().trap,
          "Privileged PC edit reports validation error without executing a trap");
  Require(machine.GetPC() == CodeAddress{3, 0x1000}, "Invalid PC write is atomic");
  machine.Reset();
  Require(machine.GetPC() == CodeAddress{}, "Reset clears PC");
  Require(Value(machine.Peek8(0x1000)) == 0xCC, "Reset preserves loaded program");
  Require(machine.Snapshot().dpp == std::array<Word, 4>{0, 1, 2, 3}, "Reset DPP identity mapping");
  Require(!machine.Read16(0x1000, AccessKind::kStack).Ok(), "Stack accesses restricted to DPRAM");
}
}  // namespace

int main() {
  try {
    RegisterAndMemoryAliases();
    SfrWidthAndSideEffects();
    AddressResolution();
    FaultsLoaderAndReset();
  } catch (const std::exception& error) {
    std::cerr << "Memory contract failed: " << error.what() << '\n';
    return 1;
  }
  return 0;
}
