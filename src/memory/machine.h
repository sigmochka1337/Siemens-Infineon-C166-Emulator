#pragma once

#include <vector>

#include "interfaces/memory.h"

namespace c166 {
// Educational C166S-derived environment; see README.md for the selected profile.
class Machine final : public IMachine {
 public:
  Machine();

  Result<Byte> Read8(PhysicalAddress address, AccessKind kind) override;
  Result<Word> Read16(PhysicalAddress address, AccessKind kind) override;
  Result<void> Write8(PhysicalAddress address, Byte value, AccessKind kind) override;
  Result<void> Write16(PhysicalAddress address, Word value, AccessKind kind) override;
  Result<Byte> Peek8(PhysicalAddress address) const override;

  Result<Word> ReadRegister(RegisterId id) override;
  Result<void> WriteRegister(RegisterId id, Word value) override;
  CodeAddress GetPC() const override;
  Result<void> SetPC(CodeAddress pc) override;
  ExtensionState GetExtension() const override;
  void SetExtension(ExtensionState state) override;

  void Reset() override;
  CpuSnapshot Snapshot() const override;
  Result<void> LoadBinary(PhysicalAddress address, std::span<const Byte> bytes) override;
  Result<PhysicalAddress> ResolveDataAddress(LogicalAddress address) const override;
  Result<PhysicalAddress> ResolveShortRegister(Byte offset, Width width) const override;
  Result<BitLocation> ResolveBitAddress(BitAddress address) const override;

 private:
  Result<void> CheckAddress(PhysicalAddress address, AccessKind kind) const;
  Result<void> CheckWord(PhysicalAddress address, AccessKind kind) const;
  Result<PhysicalAddress> RegisterAddress(RegisterId id) const;
  Result<PhysicalAddress> GprAddress(Byte offset, Width width) const;
  Word StoredWord(PhysicalAddress address) const;
  void StoreWord(PhysicalAddress address, Word value);
  Result<void> WriteSfr(PhysicalAddress address, Word value);
  void OnSoftwareRead(PhysicalAddress address);

  std::vector<Byte> storage_;
  CodeAddress pc_{};
  ExtensionState extension_{};
};
}  // namespace c166
