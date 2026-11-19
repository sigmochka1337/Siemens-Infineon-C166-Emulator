#include "executor/stack.h"

#include <algorithm>
#include <cstdint>
#include <stdexcept>

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
  Word value = machine_.ReadShortRegister(src->Id());
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
  machine_.WriteShortRegister(dst->Id(), value);
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
  if (displacement->Words() < -128 || displacement->Words() > 127) return false;

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
  if ((machine_.ReadRegister(RegisterId::kSYSCON) & syscon::kSegmentationDisabled) != 0) {
    pc.segment = 0;
  }
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

  Word value = machine_.ReadShortRegister(src->Id());
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
  if ((machine_.ReadRegister(RegisterId::kSYSCON) & syscon::kSegmentationDisabled) != 0) {
    pc.segment = 0;
  }
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
  machine_.WriteShortRegister(dst->Id(), value);
  return true;
}

// --------------------
// ------- RETI -------
// --------------------
bool Stack::Reti() {
  CodeAddress pc = machine_.GetPC();
  pc.offset = PopWord();
  if ((machine_.ReadRegister(RegisterId::kSYSCON) & syscon::kSegmentationDisabled) == 0) {
    pc.segment = static_cast<Byte>(PopWord());
  } else {
    pc.segment = 0;
  }
  Word psw_value = PopWord();
  machine_.SetPC(pc);
  machine_.WriteRegister(RegisterId::kPSW, psw_value);
  auto& frames = machine_.Cpu().trap_frames;
  if (!frames.empty()) frames.pop_back();
  return true;
}

// ---------------------------
// ------- STACK UTILS -------
// ---------------------------
void Stack::PushWord(Word value) {
  Word sp = machine_.ReadRegister(RegisterId::kSP);
  sp = static_cast<Word>(((sp - 2) & 0x0FFE) | 0xF000);
  const auto address = ResolveStackAddress(sp);
  machine_.WriteRegister(RegisterId::kSP, sp);
  machine_.Write16(address, value);
  CheckOverflow(sp);
}

Word Stack::PopWord() {
  Word sp = machine_.ReadRegister(RegisterId::kSP);
  Word value = machine_.Read16(ResolveStackAddress(sp));
  sp = static_cast<Word>(((sp + 2) & 0x0FFE) | 0xF000);
  machine_.WriteRegister(RegisterId::kSP, sp);
  CheckUnderflow(sp);
  return value;
}

Word Stack::CheckOverflow(Word sp) {
  if (sp < machine_.ReadRegister(RegisterId::kSTKOV)) {
    return RequestTrap(tfr::kStackOverflow);
  }
  return 0;
}

Word Stack::CheckUnderflow(Word sp) {
  if (sp > machine_.ReadRegister(RegisterId::kSTKUN)) {
    return RequestTrap(tfr::kStackUnderflow);
  }
  return 0;
}

PhysicalAddress Stack::ResolveStackAddress(Word sp) const {
  // System stack accesses use segment 0 and SYSCON.STKSZ, never the DPPs.
  const unsigned size = (machine_.ReadRegister(RegisterId::kSYSCON) & syscon::kStackSizeMask) >>
                        syscon::kStackSizeShift;
  Word mask;
  switch (size) {
    case 0:
      mask = 0x01FE;
      break;
    case 1:
      mask = 0x00FE;
      break;
    case 2:
      mask = 0x007E;
      break;
    case 3:
      mask = 0x003E;
      break;
    case 4:
      mask = 0x03FE;
      break;
    case 7:
      return (sp & 0x0FFE) | 0xF000;
    default:
      throw std::logic_error("Reserved SYSCON.STKSZ value");
  }
  return (0xFBFE & ~mask) | (sp & mask);
}

Word Stack::RequestTrap(Word flag) {
  const Word pending = machine_.ReadRegister(RegisterId::kTFR);
  machine_.WriteRegister(RegisterId::kTFR, pending | flag);
  return flag & ~pending;
}

void Stack::ServiceTraps(Word deferred_flags) {
  auto& cpu = machine_.Cpu();
  const Word flags = machine_.ReadRegister(RegisterId::kTFR) & ~deferred_flags;
  const bool class_b = (flags & tfr::kClassB) != 0 &&
                       std::ranges::find(cpu.trap_frames, TrapClass::kB) == cpu.trap_frames.end();
  if (std::ranges::find(cpu.trap_frames, TrapClass::kA) != cpu.trap_frames.end()) return;
  if (class_b) cpu.sequence = {};
  if (cpu.sequence.remaining != 0) return;

  if (flags & tfr::kNmi)
    EnterTrap(0x0008, TrapClass::kA);
  else if (flags & tfr::kStackOverflow)
    EnterTrap(0x0010, TrapClass::kA);
  else if (flags & tfr::kStackUnderflow)
    EnterTrap(0x0018, TrapClass::kA);
  else if (class_b)
    EnterTrap(0x0028, TrapClass::kB);
}

void Stack::EnterTrap(Word vector, TrapClass kind) {
  const Word saved_psw = machine_.ReadRegister(RegisterId::kPSW);
  const CodeAddress return_address = machine_.GetPC();
  const bool segmented =
      (machine_.ReadRegister(RegisterId::kSYSCON) & syscon::kSegmentationDisabled) == 0;
  PushWord(saved_psw);
  if (segmented) PushWord(return_address.segment);
  PushWord(return_address.offset);
  machine_.Cpu().trap_frames.push_back(kind);
  if (kind != TrapClass::kSoftware)
    machine_.WriteRegister(RegisterId::kPSW, saved_psw | psw::kPriorityMask);
  machine_.SetPC({0, vector});
}

void Stack::SetDataFlags(Word value) {
  Word psw_value = machine_.ReadRegister(RegisterId::kPSW);
  SetFlag(psw_value, psw::kE, value == 0x8000);
  SetFlag(psw_value, psw::kZ, value == 0);
  SetFlag(psw_value, psw::kN, (value & 0x8000) != 0);
  machine_.WriteRegister(RegisterId::kPSW, psw_value);
}

}  // namespace c166
