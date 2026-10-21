#pragma once

#include <cstdint>

namespace c166 {

enum class Op : std::uint16_t {
  kUnknown,
  kNop,
  kMov,
  kAdd,
  kSub,
  kPush,
  kPop,
  kCall,
  kRet,
  kJmp,
};

const char* OpName(Op op);

enum class OpKind : std::uint8_t {
  kNone,
  kReg,
  kRegByte,
  kImm,
  kMem,
  kBit,
  kRel
};

struct Operand {
  OpKind kind = OpKind::kNone;
  std::uint8_t reg = 0;
  bool high = false;
  std::uint16_t value = 0;
  std::int16_t rel = 0;
};

struct Instruction {
  Op op = Op::kUnknown;
  std::uint8_t length = 0;
  std::uint32_t pc_start = 0;
  std::uint8_t nops = 0;
  Operand ops[2]{};
};

}  // namespace c166
