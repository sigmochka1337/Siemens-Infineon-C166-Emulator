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
  void DisableWatchdog() override;
  void ServiceWatchdog() override;
  bool WatchdogResetEnabled() const { return watchdog_reset_enabled_; }
  bool WatchdogCounterEnabled() const;
  void Reset() override;
  PhysicalAddress Resolve(LogicalAddress address) override;
  void LoadBinary(PhysicalAddress address, std::span<const Byte> bytes);
  void LoadBinaryFile(PhysicalAddress address, const std::filesystem::path& path);

 private:
  void ReloadWatchdog();
  static void CheckWordAddress(PhysicalAddress address);
  void StoreByte(PhysicalAddress address, Byte value);
  PhysicalAddress RegisterAddress(RegisterId id) const;
  static bool IsWordGpr(RegisterId id);
  static bool IsByteGpr(RegisterId id);
  static PhysicalAddress SfrAddress(RegisterId id);
  IRawMemory& memory_;
  CodeAddress pc_{};
  bool watchdog_reset_enabled_ = true;
  bool watchdog_disable_locked_ = false;
};
}  // namespace c166
