#include "memory/machine.h"

#include <fstream>
#include <stdexcept>
#include <vector>

namespace c166 {
Machine::Machine(IRawMemory& memory) : memory_(memory) { Reset(); }
Byte Machine::Read8(PhysicalAddress address) const { return memory_.Read8(address); }
void Machine::Write8(PhysicalAddress address, Byte value) { memory_.Write8(address, value); }
Word Machine::Read16(PhysicalAddress address) const {
  Byte low = memory_.Read8(address);
  Byte high = memory_.Read8(address + 1);
  return static_cast<Word>(low | (high << 8));
}
void Machine::Write16(PhysicalAddress address, Word value) {
  memory_.Write8(address, static_cast<Byte>(value & 0xFF));
  memory_.Write8(address + 1, static_cast<Byte>(value >> 8));
}
bool Machine::IsWordGpr(RegisterId id) { return id >= RegisterId::kR0 && id <= RegisterId::kR15; }
bool Machine::IsByteGpr(RegisterId id) { return id >= RegisterId::kRL0 && id <= RegisterId::kRH7; }
PhysicalAddress Machine::SfrAddress(RegisterId id) {
  switch (id) {
    case RegisterId::kDPP0:
      return 0xFE00;
    case RegisterId::kDPP1:
      return 0xFE02;
    case RegisterId::kDPP2:
      return 0xFE04;
    case RegisterId::kDPP3:
      return 0xFE06;
    case RegisterId::kMDH:
      return 0xFE0C;
    case RegisterId::kMDL:
      return 0xFE0E;
    case RegisterId::kCP:
      return 0xFE10;
    case RegisterId::kSP:
      return 0xFE12;
    case RegisterId::kSTKOV:
      return 0xFE14;
    case RegisterId::kSTKUN:
      return 0xFE16;
    case RegisterId::kMDC:
      return 0xFF0E;
    case RegisterId::kPSW:
      return 0xFF10;
    default:
      throw std::invalid_argument("register is not an SFR");
  }
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
    Write8(RegisterAddress(id), static_cast<Byte>(value));
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
void Machine::Reset() {
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
  WriteRegister(RegisterId::kMDL, 0);
  WriteRegister(RegisterId::kMDH, 0);
  WriteRegister(RegisterId::kMDC, 0);
}
void Machine::LoadBinary(PhysicalAddress address, std::span<const Byte> bytes) {
  for (Byte byte : bytes) {
    Write8(address++, byte);
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
