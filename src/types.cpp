#include "types.h"
namespace c166 {
std::string MnemonicToString(Mnemonic mnemonic) {
  switch (mnemonic) {
    case Mnemonic::kNop:
      return "nop";
    case Mnemonic::kMov:
      return "mov";
    case Mnemonic::kMovB:
      return "movb";
    case Mnemonic::kAdd:
      return "add";
    case Mnemonic::kAddB:
      return "addb";
    case Mnemonic::kAddC:
      return "addc";
    case Mnemonic::kAddCB:
      return "addcb";
    case Mnemonic::kSub:
      return "sub";
    case Mnemonic::kSubB:
      return "subb";
    case Mnemonic::kSubC:
      return "subc";
    case Mnemonic::kSubCB:
      return "subcb";
    case Mnemonic::kCmp:
      return "cmp";
    case Mnemonic::kCmpB:
      return "cmpb";
    case Mnemonic::kNeg:
      return "neg";
    case Mnemonic::kNegB:
      return "negb";
    case Mnemonic::kMul:
      return "mul";
    case Mnemonic::kMulU:
      return "mulu";
    case Mnemonic::kDiv:
      return "div";
    case Mnemonic::kDivU:
      return "divu";
    case Mnemonic::kDivL:
      return "divl";
    case Mnemonic::kDivLU:
      return "divlu";
    case Mnemonic::kAnd:
      return "and";
    case Mnemonic::kAndB:
      return "andb";
    case Mnemonic::kOr:
      return "or";
    case Mnemonic::kOrB:
      return "orb";
    case Mnemonic::kXor:
      return "xor";
    case Mnemonic::kXorB:
      return "xorb";
    case Mnemonic::kCpl:
      return "cpl";
    case Mnemonic::kCplB:
      return "cplb";
    case Mnemonic::kShl:
      return "shl";
    case Mnemonic::kShr:
      return "shr";
    case Mnemonic::kAshr:
      return "ashr";
    case Mnemonic::kRol:
      return "rol";
    case Mnemonic::kRor:
      return "ror";
    case Mnemonic::kPush:
      return "push";
    case Mnemonic::kPop:
      return "pop";
    case Mnemonic::kCallA:
      return "calla";
    case Mnemonic::kCallI:
      return "calli";
    case Mnemonic::kCallR:
      return "callr";
    case Mnemonic::kCallS:
      return "calls";
    case Mnemonic::kPCall:
      return "pcall";
    case Mnemonic::kRet:
      return "ret";
    case Mnemonic::kRetS:
      return "rets";
    case Mnemonic::kRetP:
      return "retp";
    case Mnemonic::kReti:
      return "reti";
    case Mnemonic::kJmpA:
      return "jmpa";
    case Mnemonic::kJmpI:
      return "jmpi";
    case Mnemonic::kJmpR:
      return "jmpr";
    case Mnemonic::kJmpS:
      return "jmps";
    case Mnemonic::kJb:
      return "jb";
    case Mnemonic::kJnb:
      return "jnb";
    case Mnemonic::kJbc:
      return "jbc";
    case Mnemonic::kJnbs:
      return "jnbs";
    case Mnemonic::kBSet:
      return "bset";
    case Mnemonic::kBClr:
      return "bclr";
    case Mnemonic::kTrap:
      return "trap";
    case Mnemonic::kSrvwdt:
      return "srvwdt";
    case Mnemonic::kExts:
      return "exts";
    case Mnemonic::kExtp:
      return "extp";
    case Mnemonic::kExtsr:
      return "extsr";
    case Mnemonic::kExtpr:
      return "extpr";
    case Mnemonic::kExtr:
      return "extr";
    case Mnemonic::kAtomic:
      return "atomic";
    case Mnemonic::kMovBS:
      return "movbs";
    case Mnemonic::kMovBZ:
      return "movbz";
    default:
      return "unknown";
  }
}
std::string ConditionCodeToString(ConditionCode condition) {
  switch (condition) {
    case ConditionCode::kUC:
      return "cc_UC";
    case ConditionCode::kNET:
      return "cc_NET";
    case ConditionCode::kZ:
      return "cc_Z";
    case ConditionCode::kNZ:
      return "cc_NZ";
    case ConditionCode::kV:
      return "cc_V";
    case ConditionCode::kNV:
      return "cc_NV";
    case ConditionCode::kN:
      return "cc_N";
    case ConditionCode::kNN:
      return "cc_NN";
    case ConditionCode::kC:
      return "cc_C";
    case ConditionCode::kNC:
      return "cc_NC";
    case ConditionCode::kSGT:
      return "cc_SGT";
    case ConditionCode::kSLE:
      return "cc_SLE";
    case ConditionCode::kSLT:
      return "cc_SLT";
    case ConditionCode::kSGE:
      return "cc_SGE";
    case ConditionCode::kUGT:
      return "cc_UGT";
    case ConditionCode::kULE:
      return "cc_ULE";
  }
  return "cc_UC";
}

std::string RegisterIdToString(RegisterId id) {
  switch (id) {
    case RegisterId::kR0:
      return "R0";
    case RegisterId::kR1:
      return "R1";
    case RegisterId::kR2:
      return "R2";
    case RegisterId::kR3:
      return "R3";
    case RegisterId::kR4:
      return "R4";
    case RegisterId::kR5:
      return "R5";
    case RegisterId::kR6:
      return "R6";
    case RegisterId::kR7:
      return "R7";
    case RegisterId::kR8:
      return "R8";
    case RegisterId::kR9:
      return "R9";
    case RegisterId::kR10:
      return "R10";
    case RegisterId::kR11:
      return "R11";
    case RegisterId::kR12:
      return "R12";
    case RegisterId::kR13:
      return "R13";
    case RegisterId::kR14:
      return "R14";
    case RegisterId::kR15:
      return "R15";
    case RegisterId::kRL0:
      return "RL0";
    case RegisterId::kRH0:
      return "RH0";
    case RegisterId::kRL1:
      return "RL1";
    case RegisterId::kRH1:
      return "RH1";
    case RegisterId::kRL2:
      return "RL2";
    case RegisterId::kRH2:
      return "RH2";
    case RegisterId::kRL3:
      return "RL3";
    case RegisterId::kRH3:
      return "RH3";
    case RegisterId::kRL4:
      return "RL4";
    case RegisterId::kRH4:
      return "RH4";
    case RegisterId::kRL5:
      return "RL5";
    case RegisterId::kRH5:
      return "RH5";
    case RegisterId::kRL6:
      return "RL6";
    case RegisterId::kRH6:
      return "RH6";
    case RegisterId::kRL7:
      return "RL7";
    case RegisterId::kRH7:
      return "RH7";
    case RegisterId::kSP:
      return "SP";
    case RegisterId::kCP:
      return "CP";
    case RegisterId::kPSW:
      return "PSW";
    case RegisterId::kDPP0:
      return "DPP0";
    case RegisterId::kDPP1:
      return "DPP1";
    case RegisterId::kDPP2:
      return "DPP2";
    case RegisterId::kDPP3:
      return "DPP3";
    case RegisterId::kMDL:
      return "MDL";
    case RegisterId::kMDH:
      return "MDH";
    case RegisterId::kMDC:
      return "MDC";
    case RegisterId::kSTKOV:
      return "STKOV";
    case RegisterId::kSTKUN:
      return "STKUN";
    default:
      return "unknown";
  }
}

std::string Instruction::ToString() const {
  std::string res = MnemonicToString(mnemonic);
  bool first = true;

  if (condition.has_value()) {
    res += " " + ConditionCodeToString(*condition);
    first = false;
  }

  for (const auto& operand : operands) {
    res += first ? " " : ", ";
    res += operand->ToString();
    first = false;
  }
  return res;
}
}  // namespace c166
