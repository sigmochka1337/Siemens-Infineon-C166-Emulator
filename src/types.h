#pragma once

#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace c166 {
using Byte = uint8_t;
using Word = uint16_t;
using Dword = uint32_t;
using LogicalAddress = uint16_t;
using PhysicalAddress = uint32_t;
inline constexpr PhysicalAddress kMaxPhysicalAddress = 0xFFFFFF;

// CSP:IP
struct CodeAddress {
  Byte segment = 0;  // CSP
  Word offset = 0;   // IP
  bool operator==(const CodeAddress&) const = default;

  constexpr PhysicalAddress ToPhysicalAddress() const {
    return (static_cast<PhysicalAddress>(segment) << 16) | offset;
  }
};

enum class RegisterId {
  kR0,
  kR1,
  kR2,
  kR3,
  kR4,
  kR5,
  kR6,
  kR7,
  kR8,
  kR9,
  kR10,
  kR11,
  kR12,
  kR13,
  kR14,
  kR15,
  kRL0,
  kRH0,
  kRL1,
  kRH1,
  kRL2,
  kRH2,
  kRL3,
  kRH3,
  kRL4,
  kRH4,
  kRL5,
  kRH5,
  kRL6,
  kRH6,
  kRL7,
  kRH7,
  kSP,
  kCP,
  kPSW,
  kDPP0,
  kDPP1,
  kDPP2,
  kDPP3,
  kMDL,
  kMDH,
  kMDC,
  kSTKOV,
  kSTKUN,
};
namespace psw {
inline constexpr Word kN = 1u << 0;
inline constexpr Word kC = 1u << 1;
inline constexpr Word kV = 1u << 2;
inline constexpr Word kZ = 1u << 3;
inline constexpr Word kE = 1u << 4;
inline constexpr Word kMulip = 1u << 5;
inline constexpr Word kUsr0 = 1u << 6;
inline constexpr Word kConditionMask = kN | kC | kV | kZ | kE;
}  // namespace psw

enum class Mnemonic {
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

std::string MnemonicToString(Mnemonic mnemonic);
std::string RegisterIdToString(RegisterId id);
class Operand {
 public:
  virtual ~Operand() = default;
  virtual std::string ToString() const = 0;
};

struct Instruction {
  Mnemonic mnemonic;
  std::vector<std::unique_ptr<Operand>> operands;

  std::string ToString() const;
};

struct Snapshot {
  Word psw;
  std::vector<std::pair<RegisterId, Word>> registers;
};
}  // namespace c166
