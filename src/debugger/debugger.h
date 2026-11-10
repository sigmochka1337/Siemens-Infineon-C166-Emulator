#pragma once
#include <cstddef>
#include <set>
#include <string>

#include "interfaces/debugger.h"
#include "interfaces/executor.h"
#include "interfaces/fetcher.h"
#include "interfaces/memory.h"
namespace c166 {
class Debugger : public IDebugger {
 public:
  Debugger(IMachine& machine, IFetcher& fetcher, IExecutor& executor);
  void Step() override;
  void Run() override;
  void AddBreakpoint(PhysicalAddress address) override;
  void RemoveBreakpoint(PhysicalAddress address) override;
  std::string NextInstructionToString() const override;
  Snapshot Trace() const override;
  Byte PhysicalRead8(PhysicalAddress address) const override;
  Word PhysicalRead16(PhysicalAddress address) const override;
  void PhysicalWrite8(PhysicalAddress address, Byte value) override;
  void PhysicalWrite16(PhysicalAddress address, Word value) override;
  Byte LogicalRead8(LogicalAddress address) const override;
  Word LogicalRead16(LogicalAddress address) const override;
  void LogicalWrite8(LogicalAddress address, Byte value) override;
  void LogicalWrite16(LogicalAddress address, Word value) override;

 private:
  IMachine& machine_;
  IFetcher& fetcher_;
  IExecutor& executor_;
  std::set<PhysicalAddress> breakpoints_;
  std::size_t run_limit_ = 1000000;
};
}  // namespace c166
