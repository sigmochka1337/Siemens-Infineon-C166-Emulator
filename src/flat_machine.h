#pragma once

#include <cstddef>
#include <vector>

#include "interfaces/memory.h"

namespace c166 {

// Минимальная плоская машина для тестов Fetcher и демо-CLI.
//
// Это не компонент Memory: DPP/MMIO/SFR здесь не моделируются. Задача класса —
// дать Fetcher-у Read8/Read16 и загрузить в память сырой бинарник или Intel HEX.
class FlatMachine final : public IMachine {
 public:
  explicit FlatMachine(std::size_t memory_size = kMaxPhysicalAddress + 1);

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

  void ClearMemory();

 private:
  std::vector<Byte> memory_;
  std::vector<Word> registers_;
  CodeAddress pc_{};
};

}  // namespace c166
