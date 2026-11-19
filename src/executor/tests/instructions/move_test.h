#pragma once

#include "instructions/instruction_test.h"

namespace c166::test {

inline void ExecuteMove(Executor& executor, Mnemonic mnemonic, Byte size,
                        std::unique_ptr<Operand> destination, std::unique_ptr<Operand> source) {
  Instruction instruction{mnemonic, std::nullopt, {}, size};
  instruction.operands.push_back(std::move(destination));
  instruction.operands.push_back(std::move(source));
  executor.Execute(instruction);
}

}  // namespace c166::test
