#pragma once
#include <cstddef>
#include <set>

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

 private:
  IMachine& machine_;
  IFetcher& fetcher_;
  IExecutor& executor_;
  std::set<PhysicalAddress> breakpoints_;
  std::size_t run_limit_ = 1000000;
};
}  // namespace c166
