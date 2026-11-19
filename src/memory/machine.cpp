#include "memory/machine.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

#include "exceptions.h"

namespace c166 {
namespace {

struct SfrMapping {
  RegisterId id;
  PhysicalAddress address;
};

constexpr PhysicalAddress kZeros = 0xFF1C;
constexpr PhysicalAddress kOnes = 0xFF1E;
constexpr PhysicalAddress kMdl = 0xFE0E;
constexpr PhysicalAddress kMdh = 0xFE0C;
constexpr PhysicalAddress kMdc = 0xFF0E;
constexpr Byte kMdrInUse = 1u << 4;
constexpr PhysicalAddress kWdt = 0xFEAE;
constexpr PhysicalAddress kWdtCon = 0xFFAE;
constexpr PhysicalAddress kSyscon = 0xFF12;
constexpr Word kWatchdogTimerEnable = 1u << 6;

constexpr SfrMapping kSfrs[] = {
    {RegisterId::kDPP0, 0xFE00},    {RegisterId::kDPP1, 0xFE02},
    {RegisterId::kDPP2, 0xFE04},    {RegisterId::kDPP3, 0xFE06},
    {RegisterId::kMDH, 0xFE0C},     {RegisterId::kMDL, 0xFE0E},
    {RegisterId::kCP, 0xFE10},      {RegisterId::kSP, sfr::kStackPointerAddress},
    {RegisterId::kSTKOV, 0xFE14},   {RegisterId::kSTKUN, 0xFE16},
    {RegisterId::kMDC, 0xFF0E},     {RegisterId::kPSW, 0xFF10},
    {RegisterId::kSYSCON, kSyscon}, {RegisterId::kTFR, 0xFFAC},
};

}  // namespace

Machine::Machine(IRawMemory& memory) : memory_(memory) { Reset(); }
Byte Machine::Read8(PhysicalAddress address) const {
  const auto word = address & ~1u;
  if (word == kOnes) return 0xFF;
  if (word == kZeros) return 0;
  if (word == kMdl) memory_.Write8(kMdc, memory_.Read8(kMdc) & ~kMdrInUse);
  return memory_.Read8(address);
}
void Machine::Write8(PhysicalAddress address, Byte value) {
  const PhysicalAddress word_address = address & ~1u;
  if ((word_address >= 0xFE00 && word_address <= 0xFFFF) ||
      (word_address >= 0xF000 && word_address < 0xF200)) {
    // A byte write clears the other byte of an SFR (C166S V1, pp. 3-5–3-6).
    const Word word = address == word_address ? value : static_cast<Word>(value << 8);
    Write16(word_address, word);
    return;
  }
  memory_.Write8(address, value);
}
void Machine::StoreByte(PhysicalAddress address, Byte value) {
  const auto word = address & ~1u;
  if (word == kOnes || word == kZeros || word == kWdt ||
      (word == kSyscon && initialization_complete_)) {
    return;
  }
  if (word == kMdl || word == kMdh) memory_.Write8(kMdc, memory_.Read8(kMdc) | kMdrInUse);
  // SP[15:12] are fixed at 1111 and SP[0] is fixed at 0 (C166S V1, pp. 3-61–3-62).
  if (address == sfr::kStackPointerAddress) {
    value &= 0xFE;
  } else if (address == sfr::kStackPointerAddress + 1) {
    value |= 0xF0;
  }
  memory_.Write8(address, value);
}
Word Machine::Read16(PhysicalAddress address) const {
  CheckWordAddress(address);
  Byte low = Read8(address);
  Byte high = Read8(address + 1);
  return static_cast<Word>(low | (high << 8));
}
void Machine::Write16(PhysicalAddress address, Word value) {
  CheckWordAddress(address);
  StoreByte(address, static_cast<Byte>(value & 0xFF));
  StoreByte(address + 1, static_cast<Byte>(value >> 8));
}
void Machine::CheckWordAddress(PhysicalAddress address) {
  if (address > kMaxPhysicalAddress) {
    throw PhysicalAddressOutOfRange(address);
  }
  if ((address & 1u) != 0) {
    throw IllegalWordOperandAccess(address);
  }
}
bool Machine::IsWordGpr(RegisterId id) { return id >= RegisterId::kR0 && id <= RegisterId::kR15; }
bool Machine::IsByteGpr(RegisterId id) { return id >= RegisterId::kRL0 && id <= RegisterId::kRH7; }
PhysicalAddress Machine::SfrAddress(RegisterId id) {
  const auto sfr = std::ranges::find(kSfrs, id, &SfrMapping::id);
  if (sfr == std::end(kSfrs)) {
    throw std::invalid_argument("register is not an SFR");
  }
  return sfr->address;
}
PhysicalAddress Machine::RegisterAddress(RegisterId id) const {
  if (IsWordGpr(id)) {
    int n = static_cast<int>(id) - static_cast<int>(RegisterId::kR0);
    return Read16(SfrAddress(RegisterId::kCP)) + 2 * n;
  }
  if (IsByteGpr(id)) {
    int k = static_cast<int>(id) - static_cast<int>(RegisterId::kRL0);
    return Read16(SfrAddress(RegisterId::kCP)) + 2 * (k / 2) + (k % 2);
  }
  return SfrAddress(id);
}
Word Machine::ReadRegister(RegisterId id) const {
  if (IsByteGpr(id)) {
    return Read8(RegisterAddress(id));
  }
  return Read16(RegisterAddress(id));
}
void Machine::WriteRegister(RegisterId id, Word value) {
  if (IsByteGpr(id)) {
    StoreByte(RegisterAddress(id), static_cast<Byte>(value));
    return;
  }
  Write16(RegisterAddress(id), value);
}
CodeAddress Machine::GetPC() const { return pc_; }
void Machine::SetPC(CodeAddress pc) { pc_ = pc; }
PhysicalAddress Machine::Resolve(LogicalAddress address) {
  int dpp_index = address >> 14;
  RegisterId dpp = static_cast<RegisterId>(static_cast<int>(RegisterId::kDPP0) + dpp_index);
  Word page = ReadRegister(dpp) & 0x03FF;
  return (static_cast<PhysicalAddress>(page) << 14) | (address & 0x3FFF);
}
void Machine::EndInitialization() {
  initialization_complete_ = true;
  watchdog_disable_locked_ = true;
}
void Machine::DisableWatchdog() {
  if (!watchdog_disable_locked_) watchdog_reset_enabled_ = false;
}
void Machine::ServiceWatchdog() {
  ReloadWatchdog();
  watchdog_disable_locked_ = true;
}
bool Machine::WatchdogCounterEnabled() const {
  // TIMEN keeps the counter running in interrupt mode (C166S V1, pp. 9-2–9-3).
  return watchdog_reset_enabled_ || (Read16(kWdtCon) & kWatchdogTimerEnable) != 0;
}
void Machine::ReloadWatchdog() {
  memory_.Write8(kWdt, 0);
  memory_.Write8(kWdt + 1, memory_.Read8(kWdtCon + 1));
}
void Machine::Reset() {
  Cpu() = {};
  initialization_complete_ = false;
  watchdog_reset_enabled_ = true;
  watchdog_disable_locked_ = false;
  Write16(kWdtCon, 0x0081);
  ReloadWatchdog();
  pc_ = {0, 0};
  WriteRegister(RegisterId::kDPP0, 0);
  WriteRegister(RegisterId::kDPP1, 1);
  WriteRegister(RegisterId::kDPP2, 2);
  WriteRegister(RegisterId::kDPP3, 3);
  WriteRegister(RegisterId::kCP, 0xFC00);
  WriteRegister(RegisterId::kSP, 0xFC00);
  WriteRegister(RegisterId::kSTKUN, 0xFC00);
  WriteRegister(RegisterId::kSTKOV, 0xFA00);
  WriteRegister(RegisterId::kPSW, 0);
  WriteRegister(RegisterId::kSYSCON, 0);
  WriteRegister(RegisterId::kTFR, 0);
  WriteRegister(RegisterId::kMDL, 0);
  WriteRegister(RegisterId::kMDH, 0);
  WriteRegister(RegisterId::kMDC, 0);
}
void Machine::LoadBinary(PhysicalAddress address, std::span<const Byte> bytes) {
  for (Byte byte : bytes) {
    StoreByte(address++, byte);
  }
}
void Machine::LoadBinaryFile(PhysicalAddress address, const std::filesystem::path& path) {
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    throw std::runtime_error("could not open binary file: " + path.string());
  }

  const std::streamoff size = file.tellg();
  if (size < 0) {
    throw std::runtime_error("could not determine binary file size: " + path.string());
  }
  if (address > kMaxPhysicalAddress || size > kMaxPhysicalAddress - address + 1) {
    throw std::out_of_range("binary file does not fit in physical memory: " + path.string());
  }

  std::vector<Byte> bytes(static_cast<std::size_t>(size));
  file.seekg(0);
  if (!file || (!bytes.empty() && !file.read(reinterpret_cast<char*>(bytes.data()),
                                             static_cast<std::streamsize>(bytes.size())))) {
    throw std::runtime_error("could not read binary file: " + path.string());
  }
  LoadBinary(address, bytes);
}
}  // namespace c166
