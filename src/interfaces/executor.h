#pragma once

#include <array>
#include <cstdint>

#include "interfaces/decoder.h"
#include "interfaces/fetcher.h"
#include "interfaces/memory.h"

namespace c166 {
enum class CpuRunState : uint8_t {
  kRunning,
  kHalted,
  kFaulted,
};

enum class StepStatus : uint8_t {
  kCompleted,
  kHalted,
  kInvalidInstruction,
  kMemoryFault,
  kUnsupportedInstruction,
  kTrap,
};

struct FaultState {
  StepStatus status = StepStatus::kCompleted;
  PhysicalAddress address = 0;
  CodeAddress instruction_pc{};
};

struct CPUSnapshot {
  std::array<Word, 16> registers{};
  Byte csp = 0;
  Word ip = 0;
  Word sp = 0;
  std::array<Word, 4> dpp{};
  Word cp = 0;
  Word psw = 0;
  Word mdl = 0;
  Word mdh = 0;
  CpuRunState run_state = CpuRunState::kRunning;
  FaultState fault{};
};

struct ExecutionResult {
  StepStatus status = StepStatus::kCompleted;
  CodeAddress pc_before{};
  CodeAddress pc_after{};
  Instruction instruction{};
};

class IExecutor {
 public:
  virtual ExecutionResult Execute(const Instruction& instruction, IMemoryBus& mem,
                                  IRegisterBus& reg, const IConstantBus& con) const = 0;

  virtual ~IExecutor() = default;
};
}  // namespace c166
