#include "core/cpu_core.h"

#include <utility>

namespace c166 {
namespace {
// Reuse the fetch algorithm while guaranteeing debugger reads have no side effects.
class InspectionBus final : public IMemoryBus {
 public:
  explicit InspectionBus(const IMemoryBus& memory) : memory_(memory) {}
  Result<Byte> Read8(PhysicalAddress address, AccessKind) override {
    return memory_.Peek8(address);
  }
  Result<Word> Read16(PhysicalAddress address, AccessKind) override {
    if ((address & 1u) != 0) {
      return Result<Word>::Failure(
          {FaultCode::kMisaligned, address, std::nullopt, "Unaligned instruction inspection"});
    }
    auto low = memory_.Peek8(address);
    if (!low.Ok()) return Result<Word>::Failure(low.Error());
    auto high = memory_.Peek8(address + 1);
    if (!high.Ok()) return Result<Word>::Failure(high.Error());
    return Result<Word>::Success(static_cast<Word>(low.Value() | (high.Value() << 8)));
  }
  Result<void> Write8(PhysicalAddress address, Byte, AccessKind) override {
    return Result<void>::Failure(
        {FaultCode::kReadOnly, address, std::nullopt, "Instruction inspection is read-only"});
  }
  Result<void> Write16(PhysicalAddress address, Word, AccessKind) override {
    return Write8(address, 0, AccessKind::kCode);
  }
  Result<Byte> Peek8(PhysicalAddress address) const override { return memory_.Peek8(address); }

 private:
  const IMemoryBus& memory_;
};
}  // namespace

CpuCore::CpuCore(IMachine& machine, const IFetcher& fetcher, const IDecoder& decoder,
                 const IExecutor& executor)
    : machine_(machine), fetcher_(fetcher), decoder_(decoder), executor_(executor) {}

StepResult CpuCore::Fail(StepResult result, Diagnostic diagnostic, FaultStage stage) {
  result.status = IsTrap(diagnostic) ? StepStatus::kTrap : StepStatus::kFaulted;
  result.diagnostic = std::move(diagnostic);
  result.fault_stage = stage;
  result.pc_after = machine_.GetPC();
  state_ = CoreState::kFaulted;
  last_fault_ = result;
  return result;
}

StepResult CpuCore::Tick() {
  StepResult result;
  result.pc_before = machine_.GetPC();
  result.pc_after = result.pc_before;
  if (state_ == CoreState::kFaulted) {
    result.status = StepStatus::kStopped;
    result.fault_stage = FaultStage::kControl;
    result.diagnostic = Diagnostic{FaultCode::kStopped, std::nullopt, std::nullopt,
                                   "Core is faulted; Reset is required before execution"};
    return result;
  }

  auto raw = fetcher_.Fetch(result.pc_before, machine_, decoder_);
  if (!raw.Ok()) {
    return Fail(std::move(result), raw.Error(), FaultStage::kFetch);
  }
  auto instruction = decoder_.Decode(raw.Value());
  if (!instruction.Ok()) {
    return Fail(std::move(result), instruction.Error(), FaultStage::kDecode);
  }
  result.instruction = instruction.Value();
  if (instruction.Value().pc != result.pc_before ||
      instruction.Value().length != raw.Value().size) {
    return Fail(std::move(result),
                Diagnostic{FaultCode::kInvalidEncoding, std::nullopt, std::nullopt,
                           "Decoder returned an inconsistent instruction address or length"},
                FaultStage::kDecode);
  }

  const auto execution = executor_.Execute(instruction.Value(), machine_);
  result.pc_after = machine_.GetPC();
  result.flow = execution.flow;
  result.status = execution.status;
  result.diagnostic = execution.diagnostic;
  if (execution.status != StepStatus::kCompleted) {
    result.fault_stage = FaultStage::kExecute;
    state_ = CoreState::kFaulted;
    last_fault_ = result;
  }
  return result;
}

void CpuCore::Reset() {
  machine_.Reset();
  state_ = CoreState::kReady;
  last_fault_.reset();
}

Result<void> CpuCore::SetEntryPoint(CodeAddress address) { return machine_.SetPC(address); }

Result<void> CpuCore::LoadBinary(PhysicalAddress address, std::span<const Byte> bytes) {
  return machine_.LoadBinary(address, bytes);
}

Result<void> CpuCore::WriteRegister(RegisterId id, Word value) {
  return machine_.WriteRegister(id, value);
}

Result<Byte> CpuCore::PeekMemory(PhysicalAddress address) const { return machine_.Peek8(address); }

Result<Instruction> CpuCore::InspectInstruction(CodeAddress address) const {
  InspectionBus inspection(machine_);
  const auto raw = fetcher_.Fetch(address, inspection, decoder_);
  if (!raw.Ok()) return Result<Instruction>::Failure(raw.Error());
  return decoder_.Decode(raw.Value());
}

CoreSnapshot CpuCore::Snapshot() const { return {machine_.Snapshot(), state_, last_fault_}; }
}  // namespace c166
