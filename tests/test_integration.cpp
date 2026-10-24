#include <array>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include <vector>

#include "core/cpu_core.h"
#include "debugger/debugger.h"
#include "decoder/decoder.h"
#include "decoder/fetcher.h"
#include "executor/executor.h"
#include "memory/machine.h"

namespace {
using namespace c166;

void Require(bool condition, std::string_view message) {
  if (!condition) {
    std::cerr << message << '\n';
    std::exit(1);
  }
}

struct Fixture {
  Machine machine;
  Fetcher fetcher;
  Decoder decoder;
  Executor executor;
  CpuCore core{machine, fetcher, decoder, executor};
  Debugger debugger{core};
};

constexpr std::array<Byte, 12> kProgram{0xE6, 0xF0, 0x01, 0x00, 0xE6, 0xF1,
                                        0x02, 0x00, 0x00, 0x01, 0x0D, 0xFF};

class Trace final : public ITraceSink {
 public:
  void OnStep(const StepResult& step, const CoreSnapshot& after) override {
    steps.push_back(step);
    states.push_back(after);
  }
  std::vector<StepResult> steps;
  std::vector<CoreSnapshot> states;
};

void TestRealProgramAndTrace() {
  Fixture f;
  Require(f.core.LoadBinary(0, kProgram).Ok(), "load program");
  Trace trace;
  f.debugger.SetTraceSink(&trace);
  const auto run = f.debugger.Run({3, CodeAddress{0, 10}});
  f.debugger.SetTraceSink(nullptr);
  const auto snapshot = f.core.Snapshot();
  Require(run.reason == StopReason::kStopAddress, "stop address wins at exact instruction budget");
  Require(run.completed_instructions == 3, "exactly three retired instructions");
  Require(snapshot.cpu.registers[0] == 3 && snapshot.cpu.registers[1] == 2, "MOV/MOV/ADD result");
  Require(snapshot.cpu.pc == CodeAddress{0, 10}, "stop before JMPR, no double PC advance");
  Require(trace.steps.size() == 3 && trace.states.size() == 3, "one trace event per step");
  Require(trace.steps[0].pc_before.offset == 0 && trace.steps[0].pc_after.offset == 4,
          "four-byte MOV transition");
  Require(trace.states[2].cpu.registers[0] == 3, "trace observes committed state");
  Require(f.core.PeekMemory(0).Value() == 0xE6, "debugger can inspect loaded memory");
}

void TestBreakpointAndStep() {
  Fixture f;
  Require(f.core.LoadBinary(0, kProgram).Ok(), "load breakpoint program");
  f.debugger.AddBreakpoint({0, 0});
  auto run = f.debugger.Run();
  Require(run.reason == StopReason::kBreakpoint && run.completed_instructions == 0,
          "breakpoint is tested before instruction");
  run = f.debugger.Step();
  Require(run.reason == StopReason::kStepLimit && f.core.Snapshot().cpu.pc.offset == 4,
          "step ignores current breakpoint");
  f.debugger.AddBreakpoint({0, 8});
  run = f.debugger.Run({100, std::nullopt});
  Require(run.reason == StopReason::kBreakpoint && run.completed_instructions == 1,
          "run stops before ADD");
  Require(f.core.Snapshot().cpu.registers[0] == 1, "breakpoint does not execute ADD");
  f.debugger.RemoveBreakpoint({0, 8});
  run = f.debugger.Step();
  Require(run.completed_instructions == 1 && f.core.Snapshot().cpu.registers[0] == 3,
          "removed breakpoint can execute");
}

void TestLoopBudgetAndSegmentIdentity() {
  Fixture f;
  constexpr std::array<Byte, 2> kLoop{0x0D, 0xFF};
  Require(f.core.LoadBinary(0x020100, kLoop).Ok(), "load loop in nonzero segment");
  Require(f.core.SetEntryPoint({2, 0x100}).Ok(), "set segmented entry point");
  auto run = f.debugger.Run({7, std::nullopt});
  Require(run.reason == StopReason::kStepLimit && run.completed_instructions == 7,
          "self-loop ends at budget");
  Require(f.core.Snapshot().cpu.pc == CodeAddress{2, 0x100}, "relative branch preserves CSP");
  Require(run.last_step->flow == FlowKind::kBranch, "flow event is available to debugger");
  f.debugger.AddBreakpoint({1, 0x100});
  run = f.debugger.Run({1, std::nullopt});
  Require(run.completed_instructions == 1, "breakpoints include segment identity");
}

void TestFaultLatchAndRecovery() {
  Fixture f;
  // Valid MOV encoding whose SFR destination is outside this execution subset.
  constexpr std::array<Byte, 4> kUnsupported{0xE6, 0x00, 0x12, 0x34};
  Require(f.core.LoadBinary(0, kUnsupported).Ok(), "load unsupported form");
  const auto first = f.core.Tick();
  Require(first.status == StepStatus::kFaulted && first.fault_stage == FaultStage::kDecode,
          "unsupported form fails at decode");
  Require(first.diagnostic->code == FaultCode::kUnsupportedInstruction,
          "unsupported is distinct from illegal encoding");
  Require(f.core.Snapshot().cpu.pc.offset == 0, "decode failure leaves PC unchanged");
  Require(f.core.Tick().status == StepStatus::kStopped, "faulted core does not execute again");
  f.debugger.AddBreakpoint({0, 0});
  const auto stopped = f.debugger.Run({0, CodeAddress{0, 0}});
  Require(
      stopped.reason == StopReason::kFault && stopped.last_step->fault_stage == FaultStage::kDecode,
      "latched fault survives zero budget and address breakpoint");
  Require(f.core.Snapshot().last_fault->diagnostic->code == FaultCode::kUnsupportedInstruction,
          "original fault remains available");
  f.core.Reset();
  Require(f.core.Snapshot().state == CoreState::kReady, "Reset clears control fault");
  Require(f.core.PeekMemory(0).Value() == 0xE6, "Reset preserves loaded program");
  Require(f.core.LoadBinary(0, kProgram).Ok(), "replace program after reset");
  Require(f.debugger.Step().completed_instructions == 1, "execution resumes after reset");
}

void TestInspectionAndInitialization() {
  Fixture f;
  Require(f.core.LoadBinary(0, kProgram).Ok(), "load inspection program");
  const auto instruction = f.core.InspectInstruction({0, 0});
  Require(instruction.Ok() && f.decoder.Format(instruction.Value()) == "MOV R0, #0x0001",
          "inspect current instruction without executing");
  Require(f.core.Snapshot().cpu.pc.offset == 0 && f.core.Snapshot().cpu.registers[0] == 0,
          "inspection leaves CPU unchanged");
  const auto invalid = f.core.SetEntryPoint({0, 1});
  Require(!invalid.Ok() && f.core.Snapshot().state == CoreState::kReady,
          "bad debugger entry point is a rejected control request");
  const auto odd = f.core.InspectInstruction({0, 1});
  Require(!odd.Ok() && IsTrap(odd.Error()) && f.core.Snapshot().state == CoreState::kReady,
          "inspection can diagnose an odd instruction address without faulting core");
  Require(f.core.WriteRegister(RegisterId::kR0, 0x1234).Ok(), "configure initial register");
  Require(f.core.WriteRegister(RegisterId::kRH0, 0xAB).Ok(), "configure byte register alias");
  Require(f.core.Snapshot().cpu.registers[0] == 0xAB34,
          "debugger register writes use same aliases");

  Require(f.core.WriteRegister(RegisterId::kMDL, 0x00CC).Ok(), "seed MDL with NOP bytes");
  const auto before = f.core.Snapshot().cpu.mdc;
  Require(f.core.InspectInstruction({0, 0xFE0E}).Ok(),
          "disassemble bytes via read-only inspection");
  Require(f.core.Snapshot().cpu.mdc == before, "inspection does not consume MDL read effect");
}

void TestBoundaryAndIdle() {
  Fixture f;
  constexpr std::array<Byte, 2> kNop{0xCC, 0x00};
  Require(f.core.LoadBinary(0x01FFFE, kNop).Ok(), "load segment-final instruction");
  Require(f.core.SetEntryPoint({1, 0xFFFE}).Ok(), "set boundary PC");
  const auto idle = f.debugger.Step(0);
  Require(idle.completed_instructions == 0 && f.core.Snapshot().cpu.pc.offset == 0xFFFE,
          "zero steps have no execution effects");
  const auto run = f.debugger.Run({1, std::nullopt});
  Require(run.reason == StopReason::kFault && run.last_step->fault_stage == FaultStage::kExecute,
          "two-byte fetch succeeds at boundary, sequential execution is profile-limited");
  Require(run.last_step->diagnostic->code == FaultCode::kSegmentBoundary &&
              !IsTrap(*run.last_step->diagnostic),
          "profile limitation is not labelled as an architectural trap");
  Require(f.core.Snapshot().cpu.pc == CodeAddress{1, 0xFFFE}, "CSP never increments automatically");
}

void TestTrapControlContract() {
  class TrapExecutor final : public IExecutor {
   public:
    ExecutionResult Execute(const Instruction&, IMachine&) const override {
      return {StepStatus::kTrap, FlowKind::kSequential,
              Diagnostic{FaultCode::kTrap, 0, TrapKind::kSoftware, "Test trap source"}};
    }
  } executor;
  Machine machine;
  Fetcher fetcher;
  Decoder decoder;
  CpuCore core(machine, fetcher, decoder, executor);
  Debugger debugger(core);
  constexpr std::array<Byte, 2> kNop{0xCC, 0x00};
  Require(core.LoadBinary(0, kNop).Ok(), "load trap contract fixture");
  const auto run = debugger.Run();
  Require(run.reason == StopReason::kTrap && run.completed_instructions == 0,
          "debugger preserves executor trap reason");
  Require(run.last_step->diagnostic->trap == TrapKind::kSoftware &&
              core.Snapshot().state == CoreState::kFaulted,
          "trap subtype is retained and core execution is latched");
  const auto repeated = debugger.Step(0);
  Require(repeated.reason == StopReason::kTrap &&
              repeated.last_step->diagnostic->trap == TrapKind::kSoftware,
          "latched trap retains its identity even with zero step budget");
}
}  // namespace

int main() {
  TestRealProgramAndTrace();
  TestBreakpointAndStep();
  TestLoopBudgetAndSegmentIdentity();
  TestFaultLatchAndRecovery();
  TestInspectionAndInitialization();
  TestBoundaryAndIdle();
  TestTrapControlContract();
}
