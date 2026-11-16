#pragma once

#include "instructions/instruction_test.h"

namespace c166::test {

inline constexpr CodeAddress kBranchStart{2, 0x0200};
inline constexpr CodeAddress kNearTarget{2, 0x0360};
inline constexpr CodeAddress kFarTarget{3, 0x0472};
inline constexpr Word kOddTarget = 0x0361;

struct BranchConditionCase {
  const char* name;
  ConditionCode condition;
  Word flags;
  bool taken;
};

// The 16 condition codes and their PSW flag tests (pp. 5-23–5-24).
inline constexpr BranchConditionCase kBranchConditions[] = {
    {"cc_UC with clear flags", ConditionCode::kUC, 0, true},
    {"cc_UC with set flags", ConditionCode::kUC, kEndOfTable | kZero, true},
    {"cc_NET with E=0, Z=0", ConditionCode::kNET, 0, true},
    {"cc_NET with E=1", ConditionCode::kNET, kEndOfTable, false},
    {"cc_NET with Z=1", ConditionCode::kNET, kZero, false},
    {"cc_Z with Z=0", ConditionCode::kZ, 0, false},
    {"cc_Z with Z=1", ConditionCode::kZ, kZero, true},
    {"cc_NZ with Z=0", ConditionCode::kNZ, 0, true},
    {"cc_NZ with Z=1", ConditionCode::kNZ, kZero, false},
    {"cc_V with V=0", ConditionCode::kV, 0, false},
    {"cc_V with V=1", ConditionCode::kV, kOverflow, true},
    {"cc_NV with V=0", ConditionCode::kNV, 0, true},
    {"cc_NV with V=1", ConditionCode::kNV, kOverflow, false},
    {"cc_N with N=0", ConditionCode::kN, 0, false},
    {"cc_N with N=1", ConditionCode::kN, kNegative, true},
    {"cc_NN with N=0", ConditionCode::kNN, 0, true},
    {"cc_NN with N=1", ConditionCode::kNN, kNegative, false},
    {"cc_C with C=0", ConditionCode::kC, 0, false},
    {"cc_C with C=1", ConditionCode::kC, kCarry, true},
    {"cc_NC with C=0", ConditionCode::kNC, 0, true},
    {"cc_NC with C=1", ConditionCode::kNC, kCarry, false},
    {"cc_SGT with N=0, V=0, Z=0", ConditionCode::kSGT, 0, true},
    {"cc_SGT with N=1, V=1", ConditionCode::kSGT, kNegative | kOverflow, true},
    {"cc_SGT with N=1, V=0", ConditionCode::kSGT, kNegative, false},
    {"cc_SGT with N=0, V=1", ConditionCode::kSGT, kOverflow, false},
    {"cc_SGT with Z=1", ConditionCode::kSGT, kZero, false},
    {"cc_SLE with N=0, V=0, Z=0", ConditionCode::kSLE, 0, false},
    {"cc_SLE with N=1, V=1", ConditionCode::kSLE, kNegative | kOverflow, false},
    {"cc_SLE with N=1, V=0", ConditionCode::kSLE, kNegative, true},
    {"cc_SLE with N=0, V=1", ConditionCode::kSLE, kOverflow, true},
    {"cc_SLE with Z=1", ConditionCode::kSLE, kZero, true},
    {"cc_SLT with N=0, V=0", ConditionCode::kSLT, 0, false},
    {"cc_SLT with N=1, V=1", ConditionCode::kSLT, kNegative | kOverflow, false},
    {"cc_SLT with N=1, V=0", ConditionCode::kSLT, kNegative, true},
    {"cc_SLT with N=0, V=1", ConditionCode::kSLT, kOverflow, true},
    {"cc_SGE with N=0, V=0", ConditionCode::kSGE, 0, true},
    {"cc_SGE with N=1, V=1", ConditionCode::kSGE, kNegative | kOverflow, true},
    {"cc_SGE with N=1, V=0", ConditionCode::kSGE, kNegative, false},
    {"cc_SGE with N=0, V=1", ConditionCode::kSGE, kOverflow, false},
    {"cc_UGT with C=0, Z=0", ConditionCode::kUGT, 0, true},
    {"cc_UGT with C=1", ConditionCode::kUGT, kCarry, false},
    {"cc_UGT with Z=1", ConditionCode::kUGT, kZero, false},
    {"cc_UGT with C=1, Z=1", ConditionCode::kUGT, kCarry | kZero, false},
    {"cc_ULE with C=0, Z=0", ConditionCode::kULE, 0, false},
    {"cc_ULE with C=1", ConditionCode::kULE, kCarry, true},
    {"cc_ULE with Z=1", ConditionCode::kULE, kZero, true},
    {"cc_ULE with C=1, Z=1", ConditionCode::kULE, kCarry | kZero, true},
};

// ILLINA saves the odd target IP itself (p. 3-31).
inline void CheckIllegalTarget(Checks& checks, const Machine& machine, const RawMemory& raw,
                               Word target) {
  CheckHardwareTrap(checks, machine, kClassBVector, kIllegalInstruction);
  if (machine.GetPC() == kClassBVector) {
    const Word sp = machine.ReadRegister(RegisterId::kSP);
    checks.Equal("saved odd IP", ReadWord(raw, sp), target);
  }
}

}  // namespace c166::test
