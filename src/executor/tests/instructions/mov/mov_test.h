#pragma once

#include "instructions/move_test.h"

namespace c166::test::mov {

inline void ExecuteMov(Executor& executor, Byte size, std::unique_ptr<Operand> destination,
                       std::unique_ptr<Operand> source) {
  ExecuteMove(executor, Mnemonic::kMov, size, std::move(destination), std::move(source));
}

}  // namespace c166::test::mov
