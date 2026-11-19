#include "executor/bit_processor.h"

#include <optional>
#include <stdexcept>

#include "executor/binary_operand.h"
#include "executor/bit_address.h"

namespace c166 {
namespace {

void WriteData(IMachine& machine, PhysicalAddress address, Word value, bool byte) {
  if (byte) {
    machine.Write8(address, static_cast<Byte>(value));
  } else {
    machine.Write16(address, value);
  }
}

}  // namespace

void BitProcessor::SetResultFlags(Word result, Word sign_bit, Word extra_flags) {
  Word flags = (machine_.ReadRegister(RegisterId::kPSW) & ~psw::kConditionMask) | extra_flags;
  if (result == 0) flags |= psw::kZ;
  if ((result & sign_bit) != 0) flags |= psw::kN;
  machine_.WriteRegister(RegisterId::kPSW, flags);
}

Word BitProcessor::LogicalResult(Mnemonic operation, Word left, Word right, bool byte) {
  const Word sign_bit = byte ? 0x80 : 0x8000;
  if (byte) right &= 0xFF;
  Word result;
  switch (operation) {
    case Mnemonic::kAnd:
    case Mnemonic::kAndB:
      result = left & right;
      break;
    case Mnemonic::kOr:
    case Mnemonic::kOrB:
      result = left | right;
      break;
    case Mnemonic::kXor:
    case Mnemonic::kXorB:
      result = left ^ right;
      break;
    default:
      throw std::logic_error("Not a binary logical operation");
  }
  if (byte) result &= 0xFF;
  SetResultFlags(result, sign_bit, right == sign_bit ? psw::kE : 0);
  return result;
}

bool BitProcessor::Logical(Mnemonic operation, Byte size, const Operand* dst, const Operand* src) {
  const bool byte =
      operation == Mnemonic::kAndB || operation == Mnemonic::kOrB || operation == Mnemonic::kXorB;
  if (!byte && operation != Mnemonic::kAnd && operation != Mnemonic::kOr &&
      operation != Mnemonic::kXor) {
    return false;
  }

  if (const auto* mem = dynamic_cast<const DirectMemoryOperand*>(dst)) {
    if (size != 4) return false;
    PhysicalAddress address;
    Word right;
    if (const auto* src_reg = dynamic_cast<const RegisterOperand*>(src);
        src_reg && IsRegister(src_reg->Id(), byte)) {
      address = machine_.ResolveOperand(mem->Address());
      right = machine_.ReadShortRegister(src_reg->Id());
    } else if (const auto* immediate = dynamic_cast<const ImmediateOperand*>(src);
               immediate && (!byte || immediate->Value() <= 0xFF)) {
      // A decoded short SFR destination in "reg, #data" is independent of DPP.
      address = mem->Address();
      const bool sfr = address >= 0xFE00 && address <= 0xFFDE;
      const bool esfr = address >= 0xF000 && address <= 0xF1DE;
      if ((!sfr && !esfr) || (address & 1) != 0) return false;
      if (sfr && machine_.Cpu().sequence.ExtendedRegisters()) address -= 0xE00;
      right = immediate->Value();
    } else {
      return false;
    }
    const Word result = LogicalResult(operation, ReadData(machine_, address, byte), right, byte);
    WriteData(machine_, address, result, byte);
    return true;
  }

  const auto* dst_reg = dynamic_cast<const RegisterOperand*>(dst);
  if (!dst_reg || !IsRegister(dst_reg->Id(), byte)) return false;
  // Capture the old destination before [Ri+] can update an overlapping GPR.
  const Word left = machine_.ReadShortRegister(dst_reg->Id());
  const auto right = ReadBinarySource(machine_, src, size, byte, IsGpr(dst_reg->Id(), byte));
  if (!right) return false;
  machine_.WriteShortRegister(dst_reg->Id(), LogicalResult(operation, left, *right, byte));
  return true;
}

bool BitProcessor::Complement(Mnemonic operation, const Operand* dst) {
  if (operation != Mnemonic::kCpl && operation != Mnemonic::kCplB) return false;
  const bool byte = operation == Mnemonic::kCplB;
  const auto* reg = dynamic_cast<const RegisterOperand*>(dst);
  if (!reg || !IsGpr(reg->Id(), byte)) return false;

  const Word value = machine_.ReadShortRegister(reg->Id());
  const Word result = ~value & (byte ? 0xFF : 0xFFFF);
  const Word sign_bit = byte ? 0x80 : 0x8000;
  SetResultFlags(result, sign_bit, value == sign_bit ? psw::kE : 0);
  machine_.WriteShortRegister(reg->Id(), result);
  return true;
}

bool BitProcessor::Shift(Mnemonic operation, const Operand* dst, const Operand* count) {
  const auto* reg = dynamic_cast<const RegisterOperand*>(dst);
  if (!reg || !IsGpr(reg->Id(), false)) return false;

  const bool left = operation == Mnemonic::kShl || operation == Mnemonic::kRol;
  if (!left && operation != Mnemonic::kAshr && operation != Mnemonic::kShr &&
      operation != Mnemonic::kRor) {
    return false;
  }

  Word steps;
  if (const auto* immediate = dynamic_cast<const ImmediateOperand*>(count)) {
    if (immediate->Value() > 15) return false;
    steps = immediate->Value();
  } else if (const auto* src = dynamic_cast<const RegisterOperand*>(count);
             src && IsGpr(src->Id(), false)) {
    steps = machine_.ReadShortRegister(src->Id()) & 0xF;
  } else {
    return false;
  }

  Word value = machine_.ReadShortRegister(reg->Id());
  bool carry = false;
  bool overflow = false;
  for (Word i = 0; i < steps; ++i) {
    if (left) {
      carry = (value & 0x8000) != 0;
      value = static_cast<Word>(value << 1);
      if (operation == Mnemonic::kRol && carry) value |= 1;
    } else {
      // V accumulates bits shifted out before the final carry, including for ROR.
      overflow |= carry;
      carry = (value & 1) != 0;
      const bool fill = (operation == Mnemonic::kAshr && (value & 0x8000) != 0) ||
                        (operation == Mnemonic::kRor && carry);
      value = (value >> 1) | (fill ? 0x8000 : 0);
    }
  }
  SetResultFlags(value, 0x8000, (carry ? psw::kC : 0) | (overflow ? psw::kV : 0));
  machine_.WriteShortRegister(reg->Id(), value);
  return true;
}

bool BitProcessor::SetBit(const Operand* dst, bool value) {
  const auto bit = ReadAddressedBit(machine_, dst);
  if (!bit) return false;
  bit->UpdateFlags(machine_);
  bit->Write(machine_, value);
  return true;
}

bool BitProcessor::ModifyBitField(bool high_byte, const Operand* dst, const Operand* mask,
                                  const Operand* data) {
  const auto* target = dynamic_cast<const BitWordOperand*>(dst);
  const auto* mask_operand = dynamic_cast<const ImmediateOperand*>(mask);
  const auto* data_operand = dynamic_cast<const ImmediateOperand*>(data);
  if (!target || !mask_operand || !data_operand || mask_operand->Value() > 0xFF ||
      data_operand->Value() > 0xFF) {
    return false;
  }
  const auto address = ResolveBitWordAddress(machine_, target->WordAddress());
  if (!address) return false;

  const unsigned shift = high_byte ? 8 : 0;
  const Word field_mask = static_cast<Word>(mask_operand->Value() << shift);
  const Word field_data = static_cast<Word>(data_operand->Value() << shift);
  const Word result = (machine_.Read16(*address) & ~field_mask) | (field_data & field_mask);
  SetResultFlags(result, 0x8000, 0);
  machine_.Write16(*address, result);
  return true;
}

}  // namespace c166
