#pragma once

#include "executor/alu.h"
#include "executor/move_unit.h"
#include "interfaces/executor.h"
#include "interfaces/memory.h"

namespace c166 {
class Executor : public IExecutor {
 public:
  Executor(IMachine& machine) : machine_(machine), alu_(machine), move_(machine) {}

  void Execute(const Instruction& instruction) override;

 private:
  IMachine& machine_;
  ALU alu_;
  MoveUnit move_;
};
}  // namespace c166
