#pragma once
#include <filesystem>
#include <span>

#include "interfaces/memory.h"
#include "interfaces/raw_memory.h"

namespace c166 {
class Machine : public IMachine {
 public:
  explicit Machine(IRawMemory& memory);
  Byte Read8(PhysicalAddress address) const override;
  void Write8(PhysicalAddress address, Byte value) override;
  Word Read16(PhysicalAddress address) const override;
  void Write16(PhysicalAddress address, Word value) override;
  Word ReadRegister(RegisterId id) const override;
  void WriteRegister(RegisterId id, Word value) override;
  CodeAddress GetPC() const override;
  void SetPC(CodeAddress pc) override;
  void Reset() override;
  PhysicalAddress Resolve(LogicalAddress address) override;
  void LoadBinary(PhysicalAddress address, std::span<const Byte> bytes);
  void LoadBinaryFile(PhysicalAddress address, const std::filesystem::path& path);

 private:
  static void CheckWordAddress(PhysicalAddress address);
  PhysicalAddress RegisterAddress(RegisterId id) const;
  static bool IsWordGpr(RegisterId id);
  static bool IsByteGpr(RegisterId id);
  static PhysicalAddress SfrAddress(RegisterId id);
  IRawMemory& memory_;
  CodeAddress pc_{};
};
}  // namespace c166
