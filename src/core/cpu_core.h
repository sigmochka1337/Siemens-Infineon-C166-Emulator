#pragma once

#include "interfaces/cpu_core.h"
#include "interfaces/decoder.h"
#include "interfaces/executor.h"
#include "interfaces/fetcher.h"
#include "interfaces/memory.h"

namespace c166 {
// Owner: Debugger/integration. Dependencies are borrowed and must outlive the core.
class CpuCore final : public ICPUCore {
 public:
  CpuCore(IMachine& machine, const IFetcher& fetcher, const IDecoder& decoder,
          const IExecutor& executor);

  StepResult Tick() override;
  void Reset() override;
  Result<void> SetEntryPoint(CodeAddress address) override;
  Result<void> LoadBinary(PhysicalAddress address, std::span<const Byte> bytes) override;
  Result<void> WriteRegister(RegisterId id, Word value) override;
  Result<Byte> PeekMemory(PhysicalAddress address) const override;
  Result<Instruction> InspectInstruction(CodeAddress address) const override;
  CoreSnapshot Snapshot() const override;

 private:
  StepResult Fail(StepResult result, Diagnostic diagnostic, FaultStage stage);

  IMachine& machine_;
  const IFetcher& fetcher_;
  const IDecoder& decoder_;
  const IExecutor& executor_;
  CoreState state_ = CoreState::kReady;
  std::optional<StepResult> last_fault_;
};
}  // namespace c166
