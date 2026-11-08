#include "executor/stack.h"

#include <cstdint>

namespace c166 {
namespace {

bool IsConditionMet(ConditionCode condition, Word psw_value) {
  const bool e = (psw_value & psw::kE) != 0;
  const bool z = (psw_value & psw::kZ) != 0;
  const bool v = (psw_value & psw::kV) != 0;
  const bool c = (psw_value & psw::kC) != 0;
  const bool n = (psw_value & psw::kN) != 0;

  switch (condition) {
    case ConditionCode::kUC:
      return true;
    case ConditionCode::kNET:
      return !z && !e;
    case ConditionCode::kZ:
      return z;
    case ConditionCode::kNZ:
      return !z;
    case ConditionCode::kV:
      return v;
    case ConditionCode::kNV:
      return !v;
    case ConditionCode::kN:
      return n;
    case ConditionCode::kNN:
      return !n;
    case ConditionCode::kC:
      return c;
    case ConditionCode::kNC:
      return !c;
    case ConditionCode::kSGT:
      return !z && (n == v);
    case ConditionCode::kSLE:
      return z || (n != v);
    case ConditionCode::kSLT:
      return n != v;
    case ConditionCode::kSGE:
      return n == v;
    case ConditionCode::kUGT:
      return !z && !c;
    case ConditionCode::kULE:
      return z || c;
  }
  return false;
}

}  // namespace

// --------------------
// ------- PUSH -------
// --------------------
bool Stack::Push(const Operand* src) { return Push(dynamic_cast<const RegisterOperand*>(src)); }

bool Stack::Push(const RegisterOperand* src) {
  if (!src || !IsWordRegister(src->Id())) return false;

  // Capture the value before changing SP in case PUSH SP is executed.
  Word value = machine_.ReadRegister(src->Id());
  PushWord(value);
  SetDataFlags(value);
  return true;
}

// -------------------
// ------- POP -------
// -------------------
bool Stack::Pop(const Operand* dst) { return Pop(dynamic_cast<const RegisterOperand*>(dst)); }

bool Stack::Pop(const RegisterOperand* dst) {
  if (!dst || !IsWordRegister(dst->Id())) return false;

  Word value = PopWord();
  SetDataFlags(value);
  // Write last so POP SP and POP PSW follow the documented operand-write order.
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

// ---------------------
// ------- CALLA -------
// ---------------------
bool Stack::Calla(ConditionCode condition, const Operand* address) {
  return Calla(condition, dynamic_cast<const CodeAddressOperand*>(address));
}

bool Stack::Calla(ConditionCode condition, const CodeAddressOperand* address) {
  if (!address) return false;
  if (!IsConditionMet(condition, machine_.ReadRegister(RegisterId::kPSW))) return true;

  CodeAddress pc = machine_.GetPC();
  PushWord(pc.offset);
  pc.offset = address->Offset();
  machine_.SetPC(pc);
  return true;
}

// ---------------------
// ------- CALLI -------
// ---------------------
bool Stack::Calli(ConditionCode condition, const Operand* address) {
  return Calli(condition, dynamic_cast<const IndirectMemoryOperand*>(address));
}

bool Stack::Calli(ConditionCode condition, const IndirectMemoryOperand* address) {
  if (!address) return false;
  if (!IsWordGpr(address->Id())) return false;
  if (address->Mode() != IndirectMemoryOperand::AddressMode::kPlain) return false;
  if (!IsConditionMet(condition, machine_.ReadRegister(RegisterId::kPSW))) return true;

  Word target = machine_.ReadRegister(address->Id());
  CodeAddress pc = machine_.GetPC();
  PushWord(pc.offset);
  pc.offset = target;
  machine_.SetPC(pc);
  return true;
}

// ---------------------
// ------- CALLR -------
// ---------------------
bool Stack::Callr(const Operand* displacement) {
  return Callr(dynamic_cast<const RelativeOperand*>(displacement));
}

bool Stack::Callr(const RelativeOperand* displacement) {
  if (!displacement) return false;

  CodeAddress pc = machine_.GetPC();
  PushWord(pc.offset);
  pc.offset = static_cast<Word>(pc.offset + displacement->Words() * 2);
  machine_.SetPC(pc);
  return true;
}

// ---------------------
// ------- CALLS -------
// ---------------------
bool Stack::Calls(const Operand* address) {
  return Calls(dynamic_cast<const SegmentedAddressOperand*>(address));
}

bool Stack::Calls(const SegmentedAddressOperand* address) {
  if (!address) return false;

  CodeAddress pc = machine_.GetPC();
  PushWord(pc.segment);
  PushWord(pc.offset);
  pc.segment = address->Segment();
  pc.offset = address->Offset();
  machine_.SetPC(pc);
  return true;
}

// ---------------------
// ------- PCALL -------
// ---------------------
bool Stack::Pcall(const Operand* src, const Operand* address) {
  return Pcall(dynamic_cast<const RegisterOperand*>(src),
               dynamic_cast<const CodeAddressOperand*>(address));
}

bool Stack::Pcall(const RegisterOperand* src, const CodeAddressOperand* address) {
  if (!src || !address || !IsWordRegister(src->Id())) return false;

  Word value = machine_.ReadRegister(src->Id());
  CodeAddress pc = machine_.GetPC();
  PushWord(value);
  PushWord(pc.offset);
  SetDataFlags(value);
  pc.offset = address->Offset();
  machine_.SetPC(pc);
  return true;
}

// -------------------
// ------- RET -------
// -------------------
bool Stack::Ret() {
  CodeAddress pc = machine_.GetPC();
  pc.offset = PopWord();
  machine_.SetPC(pc);
  return true;
}

// --------------------
// ------- RETS -------
// --------------------
bool Stack::Rets() {
  CodeAddress pc = machine_.GetPC();
  pc.offset = PopWord();
  pc.segment = static_cast<Byte>(PopWord());
  machine_.SetPC(pc);
  return true;
}

// --------------------
// ------- RETP -------
// --------------------
bool Stack::Retp(const Operand* dst) { return Retp(dynamic_cast<const RegisterOperand*>(dst)); }

bool Stack::Retp(const RegisterOperand* dst) {
  if (!dst || !IsWordRegister(dst->Id())) return false;

  CodeAddress pc = machine_.GetPC();
  pc.offset = PopWord();
  Word value = PopWord();
  machine_.SetPC(pc);
  SetDataFlags(value);
  machine_.WriteRegister(dst->Id(), value);
  return true;
}

// --------------------
// ------- RETI -------
// --------------------
bool Stack::Reti() {
  CodeAddress pc = machine_.GetPC();
  pc.offset = PopWord();
  // The current machine model has no SYSCON.SGTDIS bit, so segmentation is
  // treated as enabled and the saved CSP is always restored.
  pc.segment = static_cast<Byte>(PopWord());
  Word psw_value = PopWord();
  machine_.SetPC(pc);
  machine_.WriteRegister(RegisterId::kPSW, psw_value);
  return true;
}

// ---------------------------
// ------- STACK UTILS -------
// ---------------------------
void Stack::PushWord(Word value) {
  Word sp = machine_.ReadRegister(RegisterId::kSP);
  sp = static_cast<Word>(sp - 2);
  machine_.WriteRegister(RegisterId::kSP, sp);
  machine_.Write16(machine_.Resolve(sp), value);
}

Word Stack::PopWord() {
  Word sp = machine_.ReadRegister(RegisterId::kSP);
  Word value = machine_.Read16(machine_.Resolve(sp));
  machine_.WriteRegister(RegisterId::kSP, static_cast<Word>(sp + 2));
  return value;
}

void Stack::SetDataFlags(Word value) {
  Word psw_value = machine_.ReadRegister(RegisterId::kPSW);
  SetFlag(psw_value, psw::kE, value == 0x8000);
  SetFlag(psw_value, psw::kZ, value == 0);
  SetFlag(psw_value, psw::kN, (value & 0x8000) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw_value);
}

}  // namespace c166
