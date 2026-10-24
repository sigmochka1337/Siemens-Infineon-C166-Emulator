#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <variant>

#include "common/types.h"

namespace c166 {
// This catalogue is not a claim that every instruction/form is implemented.
enum class Opcode : uint8_t {
  kUnknown,
  kNop,
  kMov,
  kMovB,
  kAdd,
  kAddB,
  kAddC,
  kAddCB,
  kSub,
  kSubB,
  kSubC,
  kSubCB,
  kCmp,
  kCmpB,
  kNeg,
  kNegB,
  kMul,
  kMulU,
  kDiv,
  kDivU,
  kDivL,
  kDivLU,
  kAnd,
  kAndB,
  kOr,
  kOrB,
  kXor,
  kXorB,
  kCpl,
  kCplB,
  kShl,
  kShr,
  kAshr,
  kRol,
  kRor,
  kPush,
  kPop,
  kCallA,
  kCallI,
  kCallR,
  kCallS,
  kPCall,
  kRet,
  kRetS,
  kRetP,
  kReti,
  kJmpA,
  kJmpI,
  kJmpR,
  kJmpS,
  kJb,
  kJnb,
  kJbc,
  kJnbs,
  kBSet,
  kBClr,
  kTrap,
  kSrvwdt,
  kExts,
  kExtp,
  kExtsr,
  kExtpr,
  kExtr,
  kAtomic,
};
// Numeric values are C166 condition encodings (ISM, printed page 38).
enum class Condition : uint8_t {
  kUnconditional = 0x0,
  kNotEqualAndNotEnd = 0x1,
  kZero = 0x2,
  kNotZero = 0x3,
  kOverflow = 0x4,
  kNotOverflow = 0x5,
  kNegative = 0x6,
  kNotNegative = 0x7,
  kCarry = 0x8,
  kNotCarry = 0x9,
  kSignedGreater = 0xA,
  kSignedLessOrEqual = 0xB,
  kSignedLess = 0xC,
  kSignedGreaterOrEqual = 0xD,
  kUnsignedGreater = 0xE,
  kUnsignedLessOrEqual = 0xF,
};
struct RegisterOperand {
  RegisterId id = RegisterId::kR0;
};
struct ImmediateOperand {
  Word value = 0;
};
struct DirectOperand {
  LogicalAddress address{};
};
// reg encoding selects GPR or (E)SFR; resolve using current CPU context.
struct ShortRegisterOperand {
  Byte offset = 0;
};
enum class IndirectMode : uint8_t { kPlain, kPostIncrement, kPreDecrement, kDisplacement };
struct IndirectOperand {
  RegisterId base = RegisterId::kR0;
  IndirectMode mode = IndirectMode::kPlain;
  Word displacement = 0;
};
struct RelativeOperand {
  int8_t words = 0;
};
struct CodeTargetOperand {
  Word offset = 0;
  std::optional<Byte> segment;
};
struct BitOperand {
  BitAddress address{};
};
using OperandValue = std::variant<std::monostate, RegisterOperand, ImmediateOperand, DirectOperand,
                                  ShortRegisterOperand, IndirectOperand, RelativeOperand,
                                  CodeTargetOperand, BitOperand>;
struct Operand {
  OperandValue value;
  Width width = Width::kWord;
};
struct RawInstruction {
  std::array<Byte, 4> bytes{};
  Byte size = 0;
  CodeAddress pc{};
};
struct Instruction {
  Opcode opcode = Opcode::kUnknown;
  CodeAddress pc{};
  Byte length = 0;  // A valid C166 instruction is 2 or 4 bytes.
  Condition condition = Condition::kUnconditional;
  std::array<Operand, 3> operands{};
  Byte operand_count = 0;
  std::array<Byte, 4> encoding{};
};
}  // namespace c166
