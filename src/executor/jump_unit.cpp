#include "executor/jump_unit.h"

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

bool JumpUnit::Jmpa(ConditionCode condition, const Operand* address) {
  return Jmpa(condition, dynamic_cast<const CodeAddressOperand*>(address));
}

bool JumpUnit::Jmpa(ConditionCode condition, const CodeAddressOperand* address) {
  if (!address) return false;
  if (!IsConditionMet(condition, machine_.ReadRegister(RegisterId::kPSW))) return true;

  CodeAddress pc = machine_.GetPC();
  pc.offset = address->Offset();
  machine_.SetPC(pc);
  return true;
}

bool JumpUnit::Jmpi(ConditionCode condition, const Operand* address) {
  return Jmpi(condition, dynamic_cast<const IndirectMemoryOperand*>(address));
}

bool JumpUnit::Jmpi(ConditionCode condition, const IndirectMemoryOperand* address) {
  if (!address) return false;
  if (!IsWordGpr(address->Id())) return false;
  if (address->Mode() != IndirectMemoryOperand::AddressMode::kPlain) return false;
  if (!IsConditionMet(condition, machine_.ReadRegister(RegisterId::kPSW))) return true;

  CodeAddress pc = machine_.GetPC();
  pc.offset = machine_.ReadRegister(address->Id());
  machine_.SetPC(pc);
  return true;
}

bool JumpUnit::Jmpr(ConditionCode condition, const Operand* displacement) {
  return Jmpr(condition, dynamic_cast<const RelativeOperand*>(displacement));
}

bool JumpUnit::Jmpr(ConditionCode condition, const RelativeOperand* displacement) {
  if (!displacement) return false;
  if (displacement->Words() < -128 || displacement->Words() > 127) return false;
  if (!IsConditionMet(condition, machine_.ReadRegister(RegisterId::kPSW))) return true;

  CodeAddress pc = machine_.GetPC();
  pc.offset = static_cast<Word>(pc.offset + displacement->Words() * 2);
  machine_.SetPC(pc);
  return true;
}

bool JumpUnit::Jmps(const Operand* address) {
  return Jmps(dynamic_cast<const SegmentedAddressOperand*>(address));
}

bool JumpUnit::Jmps(const SegmentedAddressOperand* address) {
  if (!address) return false;

  const bool segmented =
      (machine_.ReadRegister(RegisterId::kSYSCON) & syscon::kSegmentationDisabled) == 0;
  machine_.SetPC({segmented ? address->Segment() : Byte{0}, address->Offset()});
  return true;
}
}  // namespace c166
