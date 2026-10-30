#include "debugger/debugger.h"

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
  Instruction Instruction = fetcher_.Fetch(pc);
  executor_.Execute(Instruction);
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
}  // namespace c166