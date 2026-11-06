#include "executor/stack.h"

#include <cstdint>

namespace c166 {
namespace {

bool IsConditionMet(Word condition, Word psw_value) {
  const bool e = (psw_value & psw::kE) != 0;
  const bool z = (psw_value & psw::kZ) != 0;
  const bool v = (psw_value & psw::kV) != 0;
  const bool c = (psw_value & psw::kC) != 0;
  const bool n = (psw_value & psw::kN) != 0;

  switch (condition) {
    case 0x0:  // cc_UC
      return true;
    case 0x1:  // cc_NET
      return !z && !e;
    case 0x2:  // cc_Z / cc_EQ
      return z;
    case 0x3:  // cc_NZ / cc_NE
      return !z;
    case 0x4:  // cc_V
      return v;
    case 0x5:  // cc_NV
      return !v;
    case 0x6:  // cc_N
      return n;
    case 0x7:  // cc_NN
      return !n;
    case 0x8:  // cc_C / cc_ULT
      return c;
    case 0x9:  // cc_NC / cc_UGE
      return !c;
    case 0xA:  // cc_SGT
      return !z && (n == v);
    case 0xB:  // cc_SLE
      return z || (n != v);
    case 0xC:  // cc_SLT
      return n != v;
    case 0xD:  // cc_SGE
      return n == v;
    case 0xE:  // cc_UGT
      return !z && !c;
    case 0xF:  // cc_ULE
      return z || c;
    default:
      return false;
  }
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
bool Stack::Calla(const Operand* condition, const Operand* address) {
  return Calla(dynamic_cast<const ImmediateOperand*>(condition),
               dynamic_cast<const DirectMemoryOperand*>(address));
}

bool Stack::Calla(const ImmediateOperand* condition, const DirectMemoryOperand* address) {
  if (!condition || !address || condition->Value() > 0xF) return false;
  if (!IsConditionMet(condition->Value(), machine_.ReadRegister(RegisterId::kPSW))) return true;

  CodeAddress pc = machine_.GetPC();
  PushWord(pc.offset);
  pc.offset = address->Address();
  machine_.SetPC(pc);
  return true;
}

// ---------------------
// ------- CALLI -------
// ---------------------
bool Stack::Calli(const Operand* condition, const Operand* address) {
  return Calli(dynamic_cast<const ImmediateOperand*>(condition),
               dynamic_cast<const IndirectMemoryOperand*>(address));
}

bool Stack::Calli(const ImmediateOperand* condition, const IndirectMemoryOperand* address) {
  if (!condition || !address || condition->Value() > 0xF) return false;
  if (!IsWordGpr(address->Id())) return false;
  if (address->Mode() != IndirectMemoryOperand::AddressMode::kPlain) return false;
  if (!IsConditionMet(condition->Value(), machine_.ReadRegister(RegisterId::kPSW))) return true;

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
  return Callr(dynamic_cast<const ImmediateOperand*>(displacement));
}

bool Stack::Callr(const ImmediateOperand* displacement) {
  if (!displacement || displacement->Value() > 0xFF) return false;

  CodeAddress pc = machine_.GetPC();
  PushWord(pc.offset);
  const auto relative_words = static_cast<std::int8_t>(displacement->Value());
  pc.offset = static_cast<Word>(pc.offset + static_cast<std::int16_t>(relative_words) * 2);
  machine_.SetPC(pc);
  return true;
}

// ---------------------
// ------- CALLS -------
// ---------------------
bool Stack::Calls(const Operand* segment, const Operand* address) {
  return Calls(dynamic_cast<const ImmediateOperand*>(segment),
               dynamic_cast<const DirectMemoryOperand*>(address));
}

bool Stack::Calls(const ImmediateOperand* segment, const DirectMemoryOperand* address) {
  if (!segment || !address || segment->Value() > 0xFF) return false;

  CodeAddress pc = machine_.GetPC();
  PushWord(pc.segment);
  PushWord(pc.offset);
  pc.segment = static_cast<Byte>(segment->Value());
  pc.offset = address->Address();
  machine_.SetPC(pc);
  return true;
}

// ---------------------
// ------- PCALL -------
// ---------------------
bool Stack::Pcall(const Operand* src, const Operand* address) {
  return Pcall(dynamic_cast<const RegisterOperand*>(src),
               dynamic_cast<const DirectMemoryOperand*>(address));
}

bool Stack::Pcall(const RegisterOperand* src, const DirectMemoryOperand* address) {
  if (!src || !address || !IsWordRegister(src->Id())) return false;

  Word value = machine_.ReadRegister(src->Id());
  CodeAddress pc = machine_.GetPC();
  PushWord(value);
  PushWord(pc.offset);
  SetDataFlags(value);
  pc.offset = address->Address();
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
