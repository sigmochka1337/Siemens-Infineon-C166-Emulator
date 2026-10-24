#include "decoder/decoder.h"

#include <array>
#include <iomanip>
#include <sstream>
#include <string_view>

namespace c166 {
namespace {
enum class Form : uint8_t { kNop, kMovRegisters, kMovImmediate, kAddRegisters, kJumpRelative };
struct Encoding {
  Word mask;
  Word value;
  Byte length;
  Opcode opcode;
  Form form;
};

// Words below are little-endian: byte zero occupies bits 7..0.
// ISM printed pages: NOP 105; MOV 96; ADD 41; JMPR 91.
constexpr std::array<Encoding, 5> kEncodings{{
    {0xFFFF, 0x00CC, 2, Opcode::kNop, Form::kNop},
    {0x00FF, 0x00F0, 2, Opcode::kMov, Form::kMovRegisters},
    {0x00FF, 0x00E6, 4, Opcode::kMov, Form::kMovImmediate},
    {0x00FF, 0x0000, 2, Opcode::kAdd, Form::kAddRegisters},
    {0x000F, 0x000D, 2, Opcode::kJmpR, Form::kJumpRelative},
}};

const Encoding* FindEncoding(const std::array<Byte, 2>& first_word) {
  const Word word = static_cast<Word>(first_word[0] | (static_cast<Word>(first_word[1]) << 8));
  for (const auto& encoding : kEncodings) {
    if ((word & encoding.mask) == encoding.value) {
      return &encoding;
    }
  }
  return nullptr;
}

Diagnostic UnknownEncoding(const std::array<Byte, 2>& first_word) {
  if (first_word[0] == 0xCC) {
    return {FaultCode::kInvalidEncoding, std::nullopt, std::nullopt,
            "NOP requires the encoding CC 00"};
  }
  // Absence from this small table is not evidence of an illegal C166 opcode.
  return {FaultCode::kUnsupportedInstruction, std::nullopt, std::nullopt,
          "instruction encoding is outside the implemented decoder subset"};
}

Operand WordRegister(Byte index) {
  return {RegisterOperand{static_cast<RegisterId>(index)}, Width::kWord};
}

std::string HexWord(Word value) {
  std::ostringstream stream;
  stream << "0x" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << value;
  return stream.str();
}

std::string FormatOperand(const Operand& operand) {
  if (const auto* reg = std::get_if<RegisterOperand>(&operand.value)) {
    const auto number = static_cast<unsigned>(reg->id);
    if (number <= static_cast<unsigned>(RegisterId::kR15)) {
      return "R" + std::to_string(number);
    }
    return "<unsupported register>";
  }
  if (const auto* immediate = std::get_if<ImmediateOperand>(&operand.value)) {
    return "#" + HexWord(immediate->value);
  }
  if (const auto* relative = std::get_if<RelativeOperand>(&operand.value)) {
    return std::to_string(static_cast<int>(relative->words));
  }
  return "<unsupported operand>";
}
}  // namespace

Result<Byte> Decoder::InstructionLength(const std::array<Byte, 2>& first_word) const {
  if (const auto* encoding = FindEncoding(first_word)) {
    return Result<Byte>::Success(encoding->length);
  }
  return Result<Byte>::Failure(UnknownEncoding(first_word));
}

Result<Instruction> Decoder::Decode(const RawInstruction& raw) const {
  if (raw.size < 2) {
    return Result<Instruction>::Failure({FaultCode::kTruncatedInstruction,
                                         ToPhysicalAddress(raw.pc), std::nullopt,
                                         "instruction does not contain its first word"});
  }
  if (raw.size > raw.bytes.size()) {
    return Result<Instruction>::Failure({FaultCode::kInvalidArgument, ToPhysicalAddress(raw.pc),
                                         std::nullopt,
                                         "raw instruction exceeds its four-byte buffer"});
  }
  const std::array<Byte, 2> first_word{raw.bytes[0], raw.bytes[1]};
  const auto* encoding = FindEncoding(first_word);
  if (encoding == nullptr) {
    auto error = UnknownEncoding(first_word);
    error.address = ToPhysicalAddress(raw.pc);
    return Result<Instruction>::Failure(std::move(error));
  }
  if (raw.size < encoding->length) {
    return Result<Instruction>::Failure({FaultCode::kTruncatedInstruction,
                                         ToPhysicalAddress(raw.pc), std::nullopt,
                                         "instruction is missing its second word"});
  }
  if (raw.size != encoding->length) {
    return Result<Instruction>::Failure({FaultCode::kInvalidArgument, ToPhysicalAddress(raw.pc),
                                         std::nullopt,
                                         "raw size must equal the instruction length"});
  }

  Instruction instruction;
  instruction.opcode = encoding->opcode;
  instruction.pc = raw.pc;
  instruction.length = encoding->length;
  instruction.encoding = raw.bytes;
  switch (encoding->form) {
    case Form::kNop:
      break;
    case Form::kMovRegisters:
    case Form::kAddRegisters:
      instruction.operand_count = 2;
      instruction.operands[0] = WordRegister(static_cast<Byte>(raw.bytes[1] >> 4));
      instruction.operands[1] = WordRegister(static_cast<Byte>(raw.bytes[1] & 0x0F));
      break;
    case Form::kMovImmediate:
      if (raw.bytes[1] < 0xF0) {
        return Result<Instruction>::Failure(
            {FaultCode::kUnsupportedInstruction, ToPhysicalAddress(raw.pc), std::nullopt,
             "MOV reg,#data16 currently supports only GPR destinations"});
      }
      instruction.operand_count = 2;
      instruction.operands[0] = WordRegister(static_cast<Byte>(raw.bytes[1] & 0x0F));
      instruction.operands[1] = {ImmediateOperand{static_cast<Word>(
                                     raw.bytes[2] | (static_cast<Word>(raw.bytes[3]) << 8))},
                                 Width::kWord};
      break;
    case Form::kJumpRelative: {
      instruction.condition = static_cast<Condition>(raw.bytes[0] >> 4);
      instruction.operand_count = 1;
      // Avoid implementation-defined conversion from an out-of-range unsigned value.
      const int displacement = raw.bytes[1] < 0x80 ? raw.bytes[1] : raw.bytes[1] - 0x100;
      instruction.operands[0] = {RelativeOperand{static_cast<int8_t>(displacement)}, Width::kWord};
      break;
    }
  }
  return Result<Instruction>::Success(std::move(instruction));
}

std::string Decoder::Format(const Instruction& instruction) const {
  std::string mnemonic;
  switch (instruction.opcode) {
    case Opcode::kNop:
      mnemonic = "NOP";
      break;
    case Opcode::kMov:
      mnemonic = "MOV";
      break;
    case Opcode::kAdd:
      mnemonic = "ADD";
      break;
    case Opcode::kJmpR:
      mnemonic = "JMPR";
      break;
    default:
      return "<unsupported instruction>";
  }
  if (instruction.operand_count > instruction.operands.size()) {
    return "<invalid instruction>";
  }
  if (instruction.opcode == Opcode::kJmpR) {
    constexpr std::array<std::string_view, 16> kConditions{"UC",  "NET", "Z",   "NZ", "V",   "NV",
                                                           "N",   "NN",  "C",   "NC", "SGT", "SLE",
                                                           "SLT", "SGE", "UGT", "ULE"};
    const auto condition = static_cast<size_t>(instruction.condition);
    if (condition >= kConditions.size()) {
      return "<invalid instruction>";
    }
    mnemonic += " cc_";
    mnemonic += kConditions[condition];
    if (instruction.operand_count > 0) {
      mnemonic += ", ";
    }
  } else if (instruction.operand_count > 0) {
    mnemonic += " ";
  }
  for (Byte index = 0; index < instruction.operand_count; ++index) {
    if (index != 0) {
      mnemonic += ", ";
    }
    mnemonic += FormatOperand(instruction.operands[index]);
  }
  return mnemonic;
}
}  // namespace c166
