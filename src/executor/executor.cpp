#include "executor/executor.h"

#include "exceptions.h"
#include "operands.h"
#include "types.h"

namespace c166 {

void Executor::Execute(const Instruction& instruction) {
  switch (instruction.mnemonic) {
    case Mnemonic::kMov: {
      if (instruction.operands.size() != 2) {
        throw InvalidInstruction(instruction);
      }

      if (move_.Mov(instruction.operands[0].get(), instruction.operands[1].get())) break;
      throw InvalidInstruction(instruction);
    }

    case Mnemonic::kAdd: {
      if (instruction.operands.size() != 2) {
        throw InvalidInstruction(instruction);
      }

      if (alu_.Add(instruction.operands[0].get(), instruction.operands[1].get())) break;
      throw InvalidInstruction(instruction);
    }

    default:
      throw UnknownInstruction(instruction);
  }

  auto pc = machine_.GetPC();
  pc.offset = (pc.offset + instruction.size) & 0xFFFF;
  machine_.SetPC(pc);
}
}  // namespace c166
