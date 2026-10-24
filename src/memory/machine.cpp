#include "memory/machine.h"

#include <algorithm>
#include <array>

namespace c166 {
namespace {
constexpr PhysicalAddress kDpp0 = 0xFE00;
constexpr PhysicalAddress kMdh = 0xFE0C;
constexpr PhysicalAddress kMdl = 0xFE0E;
constexpr PhysicalAddress kCp = 0xFE10;
constexpr PhysicalAddress kSp = 0xFE12;
constexpr PhysicalAddress kStkov = 0xFE14;
constexpr PhysicalAddress kStkun = 0xFE16;
constexpr PhysicalAddress kMdc = 0xFF0E;
constexpr PhysicalAddress kPsw = 0xFF10;
constexpr PhysicalAddress kDpramStart = 0xF200;
constexpr PhysicalAddress kDpramEnd = 0xFDFF;
constexpr Word kMdriu = 0x10;

bool IsSfr(PhysicalAddress address) {
  return (address >= 0xF000 && address <= 0xF1FF) || (address >= 0xFE00 && address <= 0xFFFF);
}

bool IsImplementedSfr(PhysicalAddress address) {
  const auto word = address & ~PhysicalAddress{1};
  return (word >= kDpp0 && word <= kDpp0 + 6) || word == kMdh || word == kMdl || word == kCp ||
         word == kSp || word == kStkov || word == kStkun || word == kMdc || word == kPsw;
}

Diagnostic Error(FaultCode code, PhysicalAddress address, const char* message) {
  return {code, address, std::nullopt, message};
}

Diagnostic Invalid(const char* message) {
  return {FaultCode::kInvalidArgument, std::nullopt, std::nullopt, message};
}
}  // namespace

Machine::Machine() : storage_(static_cast<size_t>(kMaxPhysicalAddress) + 1, 0) { Reset(); }

Result<void> Machine::CheckAddress(PhysicalAddress address, AccessKind kind) const {
  if (address > kMaxPhysicalAddress) {
    return Result<void>::Failure(Error(FaultCode::kOutOfRange, address, "24-bit address required"));
  }
  if (kind == AccessKind::kStack && (address < kDpramStart || address > kDpramEnd)) {
    return Result<void>::Failure(Error(FaultCode::kUnmapped, address, "Stack must be in DPRAM"));
  }
  if (IsSfr(address)) {
    if (kind == AccessKind::kCode) {
      return Result<void>::Failure(
          Error(FaultCode::kNotExecutable, address, "SFR areas cannot contain code"));
    }
    if (!IsImplementedSfr(address)) {
      return Result<void>::Failure(
          Error(FaultCode::kUnmapped, address, "SFR is outside the implemented profile"));
    }
  }
  return Result<void>::Success();
}

Result<void> Machine::CheckWord(PhysicalAddress address, AccessKind kind) const {
  if (address > kMaxPhysicalAddress) {
    return Result<void>::Failure(Error(FaultCode::kOutOfRange, address, "24-bit address required"));
  }
  if ((address & 1) != 0) {
    return Result<void>::Failure({FaultCode::kTrap, address, TrapKind::kIllegalWordAccess,
                                  "C166 word access requires an even address"});
  }
  auto first = CheckAddress(address, kind);
  if (!first.Ok()) {
    return first;
  }
  return CheckAddress(address + 1, kind);
}

Word Machine::StoredWord(PhysicalAddress address) const {
  return static_cast<Word>(storage_[address] | (static_cast<Word>(storage_[address + 1]) << 8));
}

void Machine::StoreWord(PhysicalAddress address, Word value) {
  storage_[address] = static_cast<Byte>(value);
  storage_[address + 1] = static_cast<Byte>(value >> 8);
}

void Machine::OnSoftwareRead(PhysicalAddress address) {
  if ((address & ~PhysicalAddress{1}) == kMdl) {
    StoreWord(kMdc, static_cast<Word>(StoredWord(kMdc) & ~kMdriu));
  }
}

Result<Byte> Machine::Read8(PhysicalAddress address, AccessKind kind) {
  auto valid = CheckAddress(address, kind);
  if (!valid.Ok()) {
    return Result<Byte>::Failure(valid.Error());
  }
  const Byte value = storage_[address];
  OnSoftwareRead(address);
  return Result<Byte>::Success(value);
}

Result<Word> Machine::Read16(PhysicalAddress address, AccessKind kind) {
  auto valid = CheckWord(address, kind);
  if (!valid.Ok()) {
    return Result<Word>::Failure(valid.Error());
  }
  const Word value = StoredWord(address);
  OnSoftwareRead(address);
  return Result<Word>::Success(value);
}

Result<Byte> Machine::Peek8(PhysicalAddress address) const {
  auto valid = CheckAddress(address, AccessKind::kData);
  if (!valid.Ok()) {
    return Result<Byte>::Failure(valid.Error());
  }
  return Result<Byte>::Success(storage_[address]);
}

Result<void> Machine::WriteSfr(PhysicalAddress address, Word value) {
  if (address >= kDpp0 && address <= kDpp0 + 6) {
    value &= 0x03FF;
  } else if (address == kCp) {
    value = static_cast<Word>((value & 0x0FFE) | 0xF000);
    if ((value & 0x0E00) == 0) {
      value |= 0x0C00;
    }
    if (value < kDpramStart || static_cast<PhysicalAddress>(value) + 31 > kDpramEnd) {
      return Result<void>::Failure(
          Error(FaultCode::kUnmapped, value, "Full CP-selected register bank must fit DPRAM"));
    }
  } else if (address == kSp || address == kStkov || address == kStkun) {
    value = static_cast<Word>((value & 0x0FFE) | 0xF000);
  } else if (address == kPsw) {
    value &= 0xFC7F;
  } else if (address == kMdc) {
    value &= 0x00FF;
  }
  StoreWord(address, value);
  if (address == kMdl || address == kMdh) {
    StoreWord(kMdc, static_cast<Word>(StoredWord(kMdc) | kMdriu));
  }
  return Result<void>::Success();
}

Result<void> Machine::Write8(PhysicalAddress address, Byte value, AccessKind kind) {
  auto valid = CheckAddress(address, kind);
  if (!valid.Ok()) {
    return valid;
  }
  if (IsSfr(address)) {
    // Byte SFR stores clear the complementary byte before applying fixed bits.
    const Word word = (address & 1) == 0 ? value : static_cast<Word>(value) << 8;
    return WriteSfr(address & ~PhysicalAddress{1}, word);
  } else {
    storage_[address] = value;
  }
  return Result<void>::Success();
}

Result<void> Machine::Write16(PhysicalAddress address, Word value, AccessKind kind) {
  auto valid = CheckWord(address, kind);
  if (!valid.Ok()) {
    return valid;
  }
  if (IsSfr(address)) {
    return WriteSfr(address, value);
  } else {
    StoreWord(address, value);
  }
  return Result<void>::Success();
}

Result<PhysicalAddress> Machine::GprAddress(Byte offset, Width width) const {
  if (offset > 15 || (width != Width::kByte && width != Width::kWord)) {
    return Result<PhysicalAddress>::Failure(Invalid("Invalid GPR selector"));
  }
  const PhysicalAddress address = StoredWord(kCp) + offset * static_cast<unsigned>(width);
  const PhysicalAddress last = address + static_cast<unsigned>(width) - 1;
  if (address < kDpramStart || last > kDpramEnd) {
    return Result<PhysicalAddress>::Failure(
        Error(FaultCode::kUnmapped, address, "CP-selected GPR lies outside DPRAM"));
  }
  return Result<PhysicalAddress>::Success(address);
}

Result<PhysicalAddress> Machine::RegisterAddress(RegisterId id) const {
  const auto value = static_cast<unsigned>(id);
  if (value <= static_cast<unsigned>(RegisterId::kR15)) {
    return GprAddress(static_cast<Byte>(value), Width::kWord);
  }
  if (value <= static_cast<unsigned>(RegisterId::kRH7)) {
    return GprAddress(static_cast<Byte>(value - static_cast<unsigned>(RegisterId::kRL0)),
                      Width::kByte);
  }
  switch (id) {
    case RegisterId::kSP:
      return Result<PhysicalAddress>::Success(kSp);
    case RegisterId::kCP:
      return Result<PhysicalAddress>::Success(kCp);
    case RegisterId::kPSW:
      return Result<PhysicalAddress>::Success(kPsw);
    case RegisterId::kDPP0:
      return Result<PhysicalAddress>::Success(kDpp0);
    case RegisterId::kDPP1:
      return Result<PhysicalAddress>::Success(kDpp0 + 2);
    case RegisterId::kDPP2:
      return Result<PhysicalAddress>::Success(kDpp0 + 4);
    case RegisterId::kDPP3:
      return Result<PhysicalAddress>::Success(kDpp0 + 6);
    case RegisterId::kMDL:
      return Result<PhysicalAddress>::Success(kMdl);
    case RegisterId::kMDH:
      return Result<PhysicalAddress>::Success(kMdh);
    case RegisterId::kMDC:
      return Result<PhysicalAddress>::Success(kMdc);
    case RegisterId::kSTKOV:
      return Result<PhysicalAddress>::Success(kStkov);
    case RegisterId::kSTKUN:
      return Result<PhysicalAddress>::Success(kStkun);
    default:
      return Result<PhysicalAddress>::Failure(Invalid("Unknown register ID"));
  }
}

Result<Word> Machine::ReadRegister(RegisterId id) {
  auto address = RegisterAddress(id);
  if (!address.Ok()) {
    return Result<Word>::Failure(address.Error());
  }
  if (id >= RegisterId::kRL0 && id <= RegisterId::kRH7) {
    auto result = Read8(address.Value(), AccessKind::kData);
    return result.Ok() ? Result<Word>::Success(result.Value())
                       : Result<Word>::Failure(result.Error());
  }
  return Read16(address.Value(), AccessKind::kData);
}

Result<void> Machine::WriteRegister(RegisterId id, Word value) {
  auto address = RegisterAddress(id);
  if (!address.Ok()) {
    return Result<void>::Failure(address.Error());
  }
  if (id >= RegisterId::kRL0 && id <= RegisterId::kRH7) {
    return Write8(address.Value(), static_cast<Byte>(value), AccessKind::kData);
  }
  return Write16(address.Value(), value, AccessKind::kData);
}

CodeAddress Machine::GetPC() const { return pc_; }

Result<void> Machine::SetPC(CodeAddress pc) {
  if ((pc.offset & 1) != 0) {
    return Result<void>::Failure({FaultCode::kMisaligned, ToPhysicalAddress(pc), std::nullopt,
                                  "C166 code address requires an even offset"});
  }
  pc_ = pc;
  return Result<void>::Success();
}

ExtensionState Machine::GetExtension() const { return extension_; }
void Machine::SetExtension(ExtensionState state) { extension_ = state; }

void Machine::Reset() {
  pc_ = {};
  extension_ = {};
  for (PhysicalAddress index = 0; index < 4; ++index) {
    StoreWord(kDpp0 + 2 * index, static_cast<Word>(index));
  }
  StoreWord(kCp, 0xFC00);
  StoreWord(kSp, 0xFC00);
  StoreWord(kStkov, 0xFA00);
  StoreWord(kStkun, 0xFC00);
  StoreWord(kPsw, 0);
  StoreWord(kMdh, 0);
  StoreWord(kMdl, 0);
  StoreWord(kMdc, 0);
}

CpuSnapshot Machine::Snapshot() const {
  CpuSnapshot snapshot;
  for (Byte index = 0; index < 16; ++index) {
    snapshot.registers[index] = StoredWord(StoredWord(kCp) + 2 * index);
  }
  snapshot.pc = pc_;
  snapshot.sp = StoredWord(kSp);
  snapshot.cp = StoredWord(kCp);
  snapshot.psw = StoredWord(kPsw);
  for (size_t index = 0; index < snapshot.dpp.size(); ++index) {
    snapshot.dpp[index] = StoredWord(kDpp0 + 2 * index);
  }
  snapshot.mdl = StoredWord(kMdl);
  snapshot.mdh = StoredWord(kMdh);
  snapshot.mdc = StoredWord(kMdc);
  snapshot.stkov = StoredWord(kStkov);
  snapshot.stkun = StoredWord(kStkun);
  snapshot.extension = extension_;
  return snapshot;
}

Result<void> Machine::LoadBinary(PhysicalAddress address, std::span<const Byte> bytes) {
  if (address > kMaxPhysicalAddress || bytes.size() > storage_.size() - address) {
    return Result<void>::Failure(Error(FaultCode::kOutOfRange, address, "Binary exceeds memory"));
  }
  if (!bytes.empty()) {
    const auto last = address + bytes.size() - 1;
    if ((address <= 0xF1FF && last >= 0xF000) || (address <= 0xFFFF && last >= 0xFE00)) {
      return Result<void>::Failure(
          Error(FaultCode::kReadOnly, address, "Loader cannot initialize SFR/device regions"));
    }
  }
  std::copy(bytes.begin(), bytes.end(), storage_.begin() + address);
  return Result<void>::Success();
}

Result<PhysicalAddress> Machine::ResolveDataAddress(LogicalAddress address) const {
  if (extension_.remaining > 4) {
    return Result<PhysicalAddress>::Failure(Invalid("Extension count must be 0..4"));
  }
  if (extension_.remaining != 0 && extension_.mode == ExtensionMode::kSegment) {
    return Result<PhysicalAddress>::Success(
        (static_cast<PhysicalAddress>(extension_.value & 0xFF) << 16) | address.value);
  }
  const Word page = extension_.remaining != 0 && extension_.mode == ExtensionMode::kPage
                        ? extension_.value
                        : StoredWord(kDpp0 + 2 * (address.value >> 14));
  return Result<PhysicalAddress>::Success((static_cast<PhysicalAddress>(page & 0x3FF) << 14) |
                                          (address.value & 0x3FFF));
}

Result<PhysicalAddress> Machine::ResolveShortRegister(Byte offset, Width width) const {
  if (width != Width::kByte && width != Width::kWord) {
    return Result<PhysicalAddress>::Failure(Invalid("Invalid operand width"));
  }
  if (offset >= 0xF0) {
    return GprAddress(offset & 0x0F, width);
  }
  const PhysicalAddress base =
      extension_.remaining != 0 && extension_.extended_sfr ? 0xF000 : 0xFE00;
  return Result<PhysicalAddress>::Success(base + 2 * offset);
}

Result<BitLocation> Machine::ResolveBitAddress(BitAddress address) const {
  if (address.bit > 15) {
    return Result<BitLocation>::Failure(Invalid("Bit position must be 0..15"));
  }
  PhysicalAddress word = 0;
  if (address.offset < 0x80) {
    word = 0xFD00 + 2 * address.offset;
  } else if (address.offset < 0xF0) {
    const PhysicalAddress base =
        extension_.remaining != 0 && extension_.extended_sfr ? 0xF100 : 0xFF00;
    word = base + 2 * (address.offset & 0x7F);
  } else {
    auto result = GprAddress(address.offset & 0x0F, Width::kWord);
    if (!result.Ok()) {
      return Result<BitLocation>::Failure(result.Error());
    }
    word = result.Value();
  }
  return Result<BitLocation>::Success({word, address.bit});
}
}  // namespace c166
