#include "debugger/debugger.h"

#include <format>

#include "exceptions.h"

namespace c166 {
Debugger::Debugger(IMachine& machine, IFetcher& fetcher, IExecutor& executor)
    : machine_(machine), fetcher_(fetcher), executor_(executor) {}
void Debugger::AddBreakpoint(PhysicalAddress address) {
  if (address % 2 != 0 || address > kMaxPhysicalAddress) {
    throw InvalidBreakpointAddress(address);
  }
  breakpoints_.insert(address);
}
void Debugger::RemoveBreakpoint(PhysicalAddress address) {
  if (breakpoints_.erase(address) == 0) {
    throw BreakpointDoesNotExist(address);
  }
}
void Debugger::Step() {
  CodeAddress pc = machine_.GetPC();
  Instruction instruction = fetcher_.Fetch(pc);
  executor_.Execute(instruction);
}
void Debugger::Run() {
  for (size_t executed = 0; executed < run_limit_; ++executed) {
    PhysicalAddress address = machine_.GetPC().ToPhysicalAddress();
    if (executed > 0 && breakpoints_.contains(address)) {
      return;
    }
    Step();
  }
}
std::string Debugger::NextInstructionToString() const {
  CodeAddress pc = machine_.GetPC();
  Instruction instruction = fetcher_.Fetch(pc);
  return std::format("0x{:06X}: {}", pc.ToPhysicalAddress(), instruction.ToString());
}
Snapshot Debugger::Trace() const {
  Snapshot snapshot;
  snapshot.psw = machine_.ReadRegister(RegisterId::kPSW);
  const RegisterId registr[] = {
      RegisterId::kR0,   RegisterId::kR1,    RegisterId::kR2,    RegisterId::kR3,
      RegisterId::kR4,   RegisterId::kR5,    RegisterId::kR6,    RegisterId::kR7,
      RegisterId::kR8,   RegisterId::kR9,    RegisterId::kR10,   RegisterId::kR11,
      RegisterId::kR12,  RegisterId::kR13,   RegisterId::kR14,   RegisterId::kR15,
      RegisterId::kSP,   RegisterId::kCP,    RegisterId::kDPP0,  RegisterId::kDPP1,
      RegisterId::kDPP2, RegisterId::kDPP3,  RegisterId::kMDL,   RegisterId::kMDH,
      RegisterId::kMDC,  RegisterId::kSTKOV, RegisterId::kSTKUN,
  };
  for (RegisterId id : registr) {
    snapshot.registers.push_back({id, machine_.ReadRegister(id)});
  }
  return snapshot;
}
}  // namespace c166