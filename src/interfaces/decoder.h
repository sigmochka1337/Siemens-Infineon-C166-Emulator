#pragma once

#include <array>
#include <cstdint>

#include "interfaces/fetcher.h"
#include "interfaces/memory.h"

namespace c166 {
enum class Opcode {
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
  kCall,
  kCallA,
  kCallI,
  kCallR,
  kCallS,
  kPCall,
  kRet,
  kRetS,
  kRetP,
  kJmpA,
  kJmpI,
  kJmpR,
  kJmpS,
  kJb,
  kJnb,
  kJbc,
  kJnbs,
  kTrap,
  kSrvwdt,
  kExts,
  kExtp,
  kExtsr,
  kExtpr,
};

const char* ToString(Opcode opcode);

struct Operand {
  enum class Size { kByte, kWord, kDword };
  OperandAddress address = 0;
  Size size = Size::kWord;
};

struct Instruction {
  Opcode opcode = Opcode::kUnknown;
  CodeAddress pc{};
  uint8_t length = 0;
  std::array<Operand, 2> operands{};
  uint8_t operand_count = 0;
};

class IDecoder {
 public:
  virtual Instruction Decode(const RawInstruction& raw_instruction, const IMemoryBus& mem,
                             const IRegisterBus& reg, IConstantBus& con) = 0;

  virtual ~IDecoder() = default;
};
}  // namespace c166
