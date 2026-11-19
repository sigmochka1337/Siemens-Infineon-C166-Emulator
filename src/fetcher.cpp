#include "fetcher.h"

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

#include "exceptions.h"
#include "operands.h"
#include "short_register.h"

namespace c166 {
namespace {

// Формы операндов по C166S V1 SubSystem User's Manual, гл. 5–6.
enum class Format : std::uint8_t {
  kNone,

  kRegReg,              // dst[7:4] | src[3:0]
  kRegImm4,             // imm4[7:4] | reg[3:0]
  kRegImm16,            // reg8, data16
  kRegMem,              // reg8, mem16
  kMemReg,              // mem16, reg8
  kRegIndirect,         // reg[7:4] | base[3:0] -> reg, [Rbase]
  kRegIndirectInc,      // reg[7:4] | base[3:0] -> reg, [Rbase+]
  kMemIndirectReg,      // reg[7:4] | base[3:0] -> [Rbase], reg
  kRegIndirectDisp,     // reg[7:4] | base[3:0], d16 -> reg, [Rbase+#d16]
  kMemIndirectDispReg,  // reg[7:4] | base[3:0], d16 -> [Rbase+#d16], reg
  kMemIndirectMem,      // base[3:0], mem16 -> [Rbase], mem16
  kMemIndirectFromMem,  // base[3:0], mem16 -> mem16, [Rbase]
  kMemPreDecReg,        // reg[7:4] | base[3:0] -> [-Rbase], reg
  kMemPreDecByteReg,    // byte_reg[7:4] | base[3:0] -> [-Rbase], byte_reg
  kMemIndirectMemIndirect,      // dst_base[7:4] | src_base[3:0]
  kMemIndirectIncMemIndirect,   // dst_base[7:4] | src_base[3:0], dst+
  kMemIndirectMemIndirectInc,   // dst_base[7:4] | src_base[3:0], src+
  kShort,               // dst[7:4] | imm3/[R0..R3]/[R0..R3+][3:0]

  kByteRegReg,
  kByteRegImm4,
  kByteRegImm8,
  kByteRegMem,
  kByteMemReg,
  kByteRegIndirect,
  kByteRegIndirectInc,
  kByteMemIndirectReg,
  kByteRegIndirectDisp,
  kByteMemIndirectDispReg,
  kByteShort,

  kShift,       // count[7:4] | reg[3:0]
  kShiftReg,    // dst[7:4] | count_reg[3:0]
  kRegOnly,     // reg[7:4] | 0
  kByteRegOnly, // byte_reg[7:4] | 0
  kMul,         // dst[7:4] | src[3:0]
  kDiv,         // reg[7:4] | reg[3:0]

  kCondRel8,        // condition в opcode[7:4], rel8
  kRel8,            // rel8
  kCondCAddr,       // condition в byte1[7:4], caddr16
  kCondIndirectReg, // condition в byte1[7:4], base в byte1[3:0]
  kSegCAddr,        // seg в byte1, caddr16
  kPcall,           // reg8, caddr16
  kRetp,            // reg8
  kPushPop,         // reg8

  kBitSet,    // bit в opcode[7:4], half-word в byte1
  kBitClear,  // bit в opcode[7:4], half-word в byte1
  kBitRel,    // half-word в byte1, rel8 в byte2, bit в byte3[7:4]
  kBitField,  // bitoff в byte1, маска и данные в byte2/byte3

  kTrap,       // #trap в byte1 (номер * 2)
  kProtected,  // opcode, complement, opcode, opcode
  kReti,       // FB 88
  kExtAtomic,  // D1 <range>: EXTR/ATOMIC #1..#4
  kExtendedAddress,  // DC Rw, #range / D7 #seg|#pag, #range

  kWordByteReg,  // byte GPR[7:4] | word GPR[3:0]
};

enum class CondSource : std::uint8_t {
  kNone,
  kOpcodeHigh,
  kByte1High,
};

struct Rule {
  Byte mask = 0xFF;
  Byte value = 0;
  Mnemonic mnemonic = Mnemonic::kUnknown;
  Format format = Format::kNone;
  Byte size = 0;
  CondSource condition = CondSource::kNone;
};

std::uint8_t LowNibble(Byte value) { return value & 0x0F; }
std::uint8_t HighNibble(Byte value) { return value >> 4; }

Word WordAt(const std::array<Byte, 4>& raw, std::uint8_t offset) {
  return static_cast<Word>(raw[offset] | (raw[offset + 1] << 8));
}

RegisterId WordRegister(std::uint8_t index) {
  return static_cast<RegisterId>(static_cast<int>(RegisterId::kR0) + index);
}

RegisterId ByteRegister(std::uint8_t index) {
  return static_cast<RegisterId>(static_cast<int>(RegisterId::kRL0) + index);
}

std::unique_ptr<Operand> MakeRegister(std::uint8_t index) {
  return std::make_unique<RegisterOperand>(WordRegister(index));
}

std::unique_ptr<Operand> MakeByteRegister(std::uint8_t index) {
  return std::make_unique<RegisterOperand>(ByteRegister(index));
}

std::unique_ptr<Operand> MakeImmediate(Word value) {
  return std::make_unique<ImmediateOperand>(value);
}

std::unique_ptr<Operand> MakeDirect(Word address) {
  return std::make_unique<DirectMemoryOperand>(address);
}

std::unique_ptr<Operand> MakeIndirect(std::uint8_t base,
                                      IndirectMemoryOperand::AddressMode mode,
                                      Word displacement = 0) {
  const RegisterId id = WordRegister(base);
  switch (mode) {
    case IndirectMemoryOperand::AddressMode::kPlain:
      return IndirectMemoryOperand::CreatePlain(id);
    case IndirectMemoryOperand::AddressMode::kPostIncrement:
      return IndirectMemoryOperand::CreatePostIncrement(id);
    case IndirectMemoryOperand::AddressMode::kPreDecrement:
      return IndirectMemoryOperand::CreatePreDecrement(id);
    case IndirectMemoryOperand::AddressMode::kDisplacement:
      return IndirectMemoryOperand::CreateDisplacement(id, displacement);
  }
  throw std::logic_error("неизвестный режим косвенной адресации");
}

std::unique_ptr<Operand> MakeCodeAddress(Word offset) {
  return std::make_unique<CodeAddressOperand>(offset);
}

std::unique_ptr<Operand> MakeSegmentedAddress(Byte segment, Word offset) {
  return std::make_unique<SegmentedAddressOperand>(segment, offset);
}

std::unique_ptr<Operand> MakeRelative(std::int8_t words) {
  return std::make_unique<RelativeOperand>(words);
}

Word BitWordAddress(Byte bitoff) {
  // C166S V1, табл. 3-12. Executor выбирает SFR/ESFR и разрешает F0..FF через CP.
  if (bitoff <= 0x7F) {
    return static_cast<Word>(0xFD00u + 2u * bitoff);
  }
  return static_cast<Word>(0xFF00u + 2u * (bitoff - 0x80));
}

std::unique_ptr<Operand> MakeBit(Byte half_word, Byte bit) {
  return std::make_unique<BitOperand>(BitWordAddress(half_word), bit);
}

std::unique_ptr<Operand> MakeShortRegister(Byte address, bool byte = false) {
  return std::make_unique<RegisterOperand>(DecodeShortRegister(address, byte));
}

void AddAluFamily(std::vector<Rule>& rules, Byte base, Mnemonic mnemonic,
                  bool byte_operation, bool has_mem_destination) {
  rules.push_back({0xFF, base, mnemonic,
                   byte_operation ? Format::kByteRegReg : Format::kRegReg, 2,
                   CondSource::kNone});
  rules.push_back({0xFF, static_cast<Byte>(base + 2), mnemonic,
                   byte_operation ? Format::kByteRegMem : Format::kRegMem, 4,
                   CondSource::kNone});
  if (has_mem_destination) {
    rules.push_back({0xFF, static_cast<Byte>(base + 4), mnemonic,
                     byte_operation ? Format::kByteMemReg : Format::kMemReg, 4,
                     CondSource::kNone});
  }
  rules.push_back({0xFF, static_cast<Byte>(base + 6), mnemonic,
                   byte_operation ? Format::kByteRegImm8 : Format::kRegImm16, 4,
                   CondSource::kNone});
  rules.push_back({0xFF, static_cast<Byte>(base + 8), mnemonic,
                   byte_operation ? Format::kByteShort : Format::kShort, 2,
                   CondSource::kNone});
}

const std::vector<Rule>& Rules() {
  static const std::vector<Rule> rules = [] {
    std::vector<Rule> result;

    // Управление и системные инструкции.
    result.push_back({0x0F, 0x0D, Mnemonic::kJmpR, Format::kCondRel8, 2,
                      CondSource::kOpcodeHigh});
    result.push_back({0x0F, 0x0F, Mnemonic::kBSet, Format::kBitSet, 2,
                      CondSource::kNone});
    result.push_back({0x0F, 0x0E, Mnemonic::kBClr, Format::kBitClear, 2,
                      CondSource::kNone});

    result.push_back({0xFF, 0xBB, Mnemonic::kCallR, Format::kRel8, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xDA, Mnemonic::kCallS, Format::kSegCAddr, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xFA, Mnemonic::kJmpS, Format::kSegCAddr, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xCA, Mnemonic::kCallA, Format::kCondCAddr, 4,
                      CondSource::kByte1High});
    result.push_back({0xFF, 0xEA, Mnemonic::kJmpA, Format::kCondCAddr, 4,
                      CondSource::kByte1High});
    result.push_back({0xFF, 0xAB, Mnemonic::kCallI, Format::kCondIndirectReg, 2,
                      CondSource::kByte1High});
    result.push_back({0xFF, 0x9C, Mnemonic::kJmpI, Format::kCondIndirectReg, 2,
                      CondSource::kByte1High});
    result.push_back({0xFF, 0xE2, Mnemonic::kPCall, Format::kPcall, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xEB, Mnemonic::kRetP, Format::kRetp, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xEC, Mnemonic::kPush, Format::kPushPop, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xFC, Mnemonic::kPop, Format::kPushPop, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xCB, Mnemonic::kRet, Format::kNone, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xDB, Mnemonic::kRetS, Format::kNone, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xFB, Mnemonic::kReti, Format::kReti, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xD1, Mnemonic::kExtr, Format::kExtAtomic, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xD7, Mnemonic::kExts, Format::kExtendedAddress, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xDC, Mnemonic::kExts, Format::kExtendedAddress, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x0A, Mnemonic::kBfldl, Format::kBitField, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0x1A, Mnemonic::kBfldh, Format::kBitField, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0x8A, Mnemonic::kJb, Format::kBitRel, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0x9A, Mnemonic::kJnb, Format::kBitRel, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xAA, Mnemonic::kJbc, Format::kBitRel, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xBA, Mnemonic::kJnbs, Format::kBitRel, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0x9B, Mnemonic::kTrap, Format::kTrap, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xB5, Mnemonic::kEinit, Format::kProtected, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xA5, Mnemonic::kDiswdt, Format::kProtected, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xA7, Mnemonic::kSrvwdt, Format::kProtected, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xCC, Mnemonic::kNop, Format::kNone, 2,
                      CondSource::kNone});

    // ALU.
    AddAluFamily(result, 0x00, Mnemonic::kAdd, false, true);
    AddAluFamily(result, 0x10, Mnemonic::kAddC, false, true);
    AddAluFamily(result, 0x20, Mnemonic::kSub, false, true);
    AddAluFamily(result, 0x30, Mnemonic::kSubC, false, true);
    AddAluFamily(result, 0x40, Mnemonic::kCmp, false, false);
    AddAluFamily(result, 0x01, Mnemonic::kAddB, true, true);
    AddAluFamily(result, 0x11, Mnemonic::kAddCB, true, true);
    AddAluFamily(result, 0x21, Mnemonic::kSubB, true, true);
    AddAluFamily(result, 0x31, Mnemonic::kSubCB, true, true);
    AddAluFamily(result, 0x41, Mnemonic::kCmpB, true, false);

    // BitProcessor: AND/OR/XOR и их байтовые версии.
    AddAluFamily(result, 0x60, Mnemonic::kAnd, false, true);
    AddAluFamily(result, 0x70, Mnemonic::kOr, false, true);
    AddAluFamily(result, 0x50, Mnemonic::kXor, false, true);
    AddAluFamily(result, 0x61, Mnemonic::kAndB, true, true);
    AddAluFamily(result, 0x71, Mnemonic::kOrB, true, true);
    AddAluFamily(result, 0x51, Mnemonic::kXorB, true, true);

    // CPL/NEG.
    result.push_back({0xFF, 0x91, Mnemonic::kCpl, Format::kRegOnly, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xB1, Mnemonic::kCplB, Format::kByteRegOnly, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x81, Mnemonic::kNeg, Format::kRegOnly, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xA1, Mnemonic::kNegB, Format::kByteRegOnly, 2,
                      CondSource::kNone});

    // Умножение/деление.
    result.push_back({0xFF, 0x0B, Mnemonic::kMul, Format::kMul, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x1B, Mnemonic::kMulU, Format::kMul, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x4B, Mnemonic::kDiv, Format::kDiv, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x5B, Mnemonic::kDivU, Format::kDiv, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x6B, Mnemonic::kDivL, Format::kDiv, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x7B, Mnemonic::kDivLU, Format::kDiv, 2,
                      CondSource::kNone});

    // Сдвиги и вращения.
    result.push_back({0xFF, 0x0C, Mnemonic::kRol, Format::kShiftReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x2C, Mnemonic::kRor, Format::kShiftReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x4C, Mnemonic::kShl, Format::kShiftReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x6C, Mnemonic::kShr, Format::kShiftReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xAC, Mnemonic::kAshr, Format::kShiftReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x5C, Mnemonic::kShl, Format::kShift, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x7C, Mnemonic::kShr, Format::kShift, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xBC, Mnemonic::kAshr, Format::kShift, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x1C, Mnemonic::kRol, Format::kShift, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x3C, Mnemonic::kRor, Format::kShift, 2,
                      CondSource::kNone});

    // MoveUnit: MOV.
    result.push_back({0xFF, 0xE0, Mnemonic::kMov, Format::kRegImm4, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xE6, Mnemonic::kMov, Format::kRegImm16, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xF0, Mnemonic::kMov, Format::kRegReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xA8, Mnemonic::kMov, Format::kRegIndirect, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x98, Mnemonic::kMov, Format::kRegIndirectInc, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xB8, Mnemonic::kMov, Format::kMemIndirectReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xD4, Mnemonic::kMov, Format::kRegIndirectDisp, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xC4, Mnemonic::kMov, Format::kMemIndirectDispReg, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xF2, Mnemonic::kMov, Format::kRegMem, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xF6, Mnemonic::kMov, Format::kMemReg, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0x84, Mnemonic::kMov, Format::kMemIndirectMem, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0x94, Mnemonic::kMov, Format::kMemIndirectFromMem, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0x88, Mnemonic::kMov, Format::kMemPreDecReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x89, Mnemonic::kMovB, Format::kMemPreDecByteReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xC8, Mnemonic::kMov,
                      Format::kMemIndirectMemIndirect, 2, CondSource::kNone});
    result.push_back({0xFF, 0xD8, Mnemonic::kMov,
                      Format::kMemIndirectIncMemIndirect, 2, CondSource::kNone});
    result.push_back({0xFF, 0xE8, Mnemonic::kMov, Format::kMemIndirectMemIndirectInc, 2,
                      CondSource::kNone});

    // MoveUnit: MOVB.
    result.push_back({0xFF, 0xA4, Mnemonic::kMovB, Format::kMemIndirectMem, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xB4, Mnemonic::kMovB, Format::kMemIndirectFromMem, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xC9, Mnemonic::kMovB, Format::kMemIndirectMemIndirect, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xD9, Mnemonic::kMovB, Format::kMemIndirectIncMemIndirect, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xE9, Mnemonic::kMovB, Format::kMemIndirectMemIndirectInc, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xE1, Mnemonic::kMovB, Format::kByteRegImm4, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xE7, Mnemonic::kMovB, Format::kByteRegImm8, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xF1, Mnemonic::kMovB, Format::kByteRegReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xA9, Mnemonic::kMovB, Format::kByteRegIndirect, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0x99, Mnemonic::kMovB, Format::kByteRegIndirectInc, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xB9, Mnemonic::kMovB, Format::kByteMemIndirectReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xF4, Mnemonic::kMovB, Format::kByteRegIndirectDisp, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xE4, Mnemonic::kMovB, Format::kByteMemIndirectDispReg, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xF3, Mnemonic::kMovB, Format::kByteRegMem, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xF7, Mnemonic::kMovB, Format::kByteMemReg, 4,
                      CondSource::kNone});

    // MoveUnit: MOVBS/MOVBZ.
    result.push_back({0xFF, 0xD0, Mnemonic::kMovBS, Format::kWordByteReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xC0, Mnemonic::kMovBZ, Format::kWordByteReg, 2,
                      CondSource::kNone});
    result.push_back({0xFF, 0xD2, Mnemonic::kMovBS, Format::kRegMem, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xC2, Mnemonic::kMovBZ, Format::kRegMem, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xD5, Mnemonic::kMovBS, Format::kByteMemReg, 4,
                      CondSource::kNone});
    result.push_back({0xFF, 0xC5, Mnemonic::kMovBZ, Format::kByteMemReg, 4,
                      CondSource::kNone});

    return result;
  }();
  return rules;
}

const Rule* FindRule(Byte opcode) {
  for (const Rule& rule : Rules()) {
    if ((opcode & rule.mask) == rule.value) {
      return &rule;
    }
  }
  return nullptr;
}

Byte ReadByte(const IMachine& machine, CodeAddress pc, std::uint8_t offset,
              std::array<Byte, 4>& raw) {
  const std::uint32_t instruction_offset =
      static_cast<std::uint32_t>(pc.offset) + offset;
  if (instruction_offset > 0xFFFF) {
    throw UnreadableInstruction(pc, raw);
  }
  const PhysicalAddress address =
      (static_cast<PhysicalAddress>(pc.segment) << 16) | instruction_offset;
  try {
    raw[offset] = machine.Read8(address);
  } catch (const PhysicalAddressOutOfRange&) {
    throw UnreadableInstruction(pc, raw);
  }
  return raw[offset];
}

[[noreturn]] void Invalid(CodeAddress pc, const std::array<Byte, 4>& raw) {
  throw UnreadableInstruction(pc, raw);
}

Instruction MakeInstruction(Mnemonic mnemonic, Byte size) {
  Instruction instruction;
  instruction.mnemonic = mnemonic;
  instruction.size = size;
  return instruction;
}

std::unique_ptr<Operand> MakeBitOperand(Byte half_word, Byte bit) {
  return MakeBit(half_word, bit);
}

void SetCondition(Instruction& instruction, CondSource source, Byte opcode,
                 Byte byte1) {
  switch (source) {
    case CondSource::kNone:
      return;
    case CondSource::kOpcodeHigh:
      instruction.condition = static_cast<ConditionCode>(opcode >> 4);
      return;
    case CondSource::kByte1High:
      instruction.condition = static_cast<ConditionCode>(byte1 >> 4);
      return;
  }
}

}  // namespace

Instruction Fetcher::Fetch(CodeAddress pc) {
  std::array<Byte, 4> raw{};

  const Byte opcode = ReadByte(machine_, pc, 0, raw);
  ReadByte(machine_, pc, 1, raw);  // минимальная инструкция C166 — 2 байта

  const Rule* rule = FindRule(opcode);
  if (rule == nullptr || rule->mnemonic == Mnemonic::kUnknown || rule->size == 0) {
    throw UnreadableInstruction(pc, raw);
  }

  if (rule->size == 4) {
    ReadByte(machine_, pc, 2, raw);
    ReadByte(machine_, pc, 3, raw);
  }

  const Byte byte1 = raw[1];
  const Byte high = HighNibble(byte1);
  const Byte low = LowNibble(byte1);
  const Word word = WordAt(raw, 2);

  Instruction instruction = MakeInstruction(rule->mnemonic, rule->size);
  SetCondition(instruction, rule->condition, opcode, byte1);

  switch (rule->format) {
    case Format::kNone:
      // NOP, RET, RETS: второй байт фиксирован нулём.
      if (byte1 != 0) {
        Invalid(pc, raw);
      }
      return instruction;

    case Format::kRegReg:
      instruction.operands.push_back(MakeRegister(high));
      instruction.operands.push_back(MakeRegister(low));
      return instruction;

    case Format::kRegImm4:
      instruction.operands.push_back(MakeRegister(low));
      instruction.operands.push_back(MakeImmediate(high));
      return instruction;

    case Format::kRegImm16:
      instruction.operands.push_back(MakeShortRegister(byte1));
      instruction.operands.push_back(MakeImmediate(word));
      return instruction;

    case Format::kRegMem:
      instruction.operands.push_back(MakeShortRegister(byte1));
      instruction.operands.push_back(MakeDirect(word));
      return instruction;

    case Format::kMemReg:
      instruction.operands.push_back(MakeDirect(word));
      instruction.operands.push_back(MakeShortRegister(byte1));
      return instruction;

    case Format::kRegIndirect:
      instruction.operands.push_back(MakeRegister(high));
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      return instruction;

    case Format::kRegIndirectInc:
      instruction.operands.push_back(MakeRegister(high));
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPostIncrement));
      return instruction;

    case Format::kMemIndirectReg:
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      instruction.operands.push_back(MakeRegister(high));
      return instruction;

    case Format::kRegIndirectDisp:
      instruction.operands.push_back(MakeRegister(high));
      instruction.operands.push_back(MakeIndirect(
          low, IndirectMemoryOperand::AddressMode::kDisplacement, word));
      return instruction;

    case Format::kMemIndirectDispReg:
      instruction.operands.push_back(MakeIndirect(
          low, IndirectMemoryOperand::AddressMode::kDisplacement, word));
      instruction.operands.push_back(MakeRegister(high));
      return instruction;

    case Format::kMemIndirectMem:
      if (high != 0) {
        Invalid(pc, raw);
      }
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      instruction.operands.push_back(MakeDirect(word));
      return instruction;

    case Format::kMemIndirectFromMem:
      if (high != 0) {
        Invalid(pc, raw);
      }
      instruction.operands.push_back(MakeDirect(word));
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      return instruction;

    case Format::kMemPreDecReg:
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPreDecrement));
      instruction.operands.push_back(MakeRegister(high));
      return instruction;

    case Format::kMemPreDecByteReg:
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPreDecrement));
      instruction.operands.push_back(MakeByteRegister(high));
      return instruction;

    case Format::kMemIndirectMemIndirect:
      instruction.operands.push_back(
          MakeIndirect(high, IndirectMemoryOperand::AddressMode::kPlain));
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      return instruction;

    case Format::kMemIndirectIncMemIndirect:
      instruction.operands.push_back(
          MakeIndirect(high, IndirectMemoryOperand::AddressMode::kPostIncrement));
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      return instruction;

    case Format::kMemIndirectMemIndirectInc:
      instruction.operands.push_back(
          MakeIndirect(high, IndirectMemoryOperand::AddressMode::kPlain));
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPostIncrement));
      return instruction;

    case Format::kShort:
    case Format::kByteShort: {
      const bool byte_operation = rule->format == Format::kByteShort;
      instruction.operands.push_back(byte_operation ? MakeByteRegister(high)
                                                    : MakeRegister(high));
      if (low <= 7) {
        instruction.operands.push_back(MakeImmediate(low));
      } else if (low <= 11) {
        instruction.operands.push_back(
            MakeIndirect(static_cast<Byte>(low - 8),
                         IndirectMemoryOperand::AddressMode::kPlain));
      } else {
        instruction.operands.push_back(
            MakeIndirect(static_cast<Byte>(low - 12),
                         IndirectMemoryOperand::AddressMode::kPostIncrement));
      }
      return instruction;
    }

    case Format::kByteRegReg:
      instruction.operands.push_back(MakeByteRegister(high));
      instruction.operands.push_back(MakeByteRegister(low));
      return instruction;

    case Format::kByteRegImm4:
      instruction.operands.push_back(MakeByteRegister(low));
      instruction.operands.push_back(MakeImmediate(high));
      return instruction;

    case Format::kByteRegImm8:
      instruction.operands.push_back(MakeShortRegister(byte1, true));
      instruction.operands.push_back(MakeImmediate(raw[2]));
      return instruction;

    case Format::kByteRegMem:
      instruction.operands.push_back(MakeShortRegister(byte1, true));
      instruction.operands.push_back(MakeDirect(word));
      return instruction;

    case Format::kByteMemReg:
      instruction.operands.push_back(MakeDirect(word));
      instruction.operands.push_back(MakeShortRegister(byte1, true));
      return instruction;

    case Format::kByteRegIndirect:
      instruction.operands.push_back(MakeByteRegister(high));
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      return instruction;

    case Format::kByteRegIndirectInc:
      instruction.operands.push_back(MakeByteRegister(high));
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPostIncrement));
      return instruction;

    case Format::kByteMemIndirectReg:
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      instruction.operands.push_back(MakeByteRegister(high));
      return instruction;

    case Format::kByteRegIndirectDisp:
      instruction.operands.push_back(MakeByteRegister(high));
      instruction.operands.push_back(MakeIndirect(
          low, IndirectMemoryOperand::AddressMode::kDisplacement, word));
      return instruction;

    case Format::kByteMemIndirectDispReg:
      instruction.operands.push_back(MakeIndirect(
          low, IndirectMemoryOperand::AddressMode::kDisplacement, word));
      instruction.operands.push_back(MakeByteRegister(high));
      return instruction;

    case Format::kShift:
      instruction.operands.push_back(MakeRegister(low));
      instruction.operands.push_back(MakeImmediate(high));
      return instruction;

    case Format::kShiftReg:
      instruction.operands.push_back(MakeRegister(high));
      instruction.operands.push_back(MakeRegister(low));
      return instruction;

    case Format::kRegOnly:
      if (low != 0) {
        Invalid(pc, raw);
      }
      instruction.operands.push_back(MakeRegister(high));
      return instruction;

    case Format::kByteRegOnly:
      if (low != 0) {
        Invalid(pc, raw);
      }
      instruction.operands.push_back(MakeByteRegister(high));
      return instruction;

    case Format::kMul:
      instruction.operands.push_back(MakeRegister(high));
      instruction.operands.push_back(MakeRegister(low));
      return instruction;

    case Format::kDiv:
      if (high != low) {
        Invalid(pc, raw);
      }
      instruction.operands.push_back(MakeRegister(high));
      return instruction;

    case Format::kCondRel8:
      instruction.operands.push_back(MakeRelative(static_cast<std::int8_t>(byte1)));
      return instruction;

    case Format::kRel8:
      instruction.operands.push_back(MakeRelative(static_cast<std::int8_t>(byte1)));
      return instruction;

    case Format::kCondCAddr:
      if (low != 0) {
        Invalid(pc, raw);
      }
      instruction.operands.push_back(MakeCodeAddress(word));
      return instruction;

    case Format::kCondIndirectReg:
      instruction.operands.push_back(
          MakeIndirect(low, IndirectMemoryOperand::AddressMode::kPlain));
      return instruction;

    case Format::kSegCAddr:
      instruction.operands.push_back(MakeSegmentedAddress(byte1, word));
      return instruction;

    case Format::kPcall:
    case Format::kRetp:
    case Format::kPushPop:
      instruction.operands.push_back(MakeShortRegister(byte1));
      if (rule->format == Format::kPcall) {
        instruction.operands.push_back(MakeCodeAddress(word));
      }
      return instruction;

    case Format::kBitSet:
    case Format::kBitClear:
      instruction.operands.push_back(
          MakeBitOperand(byte1, static_cast<Byte>(opcode >> 4)));
      return instruction;

    case Format::kBitField: {
      const bool high_byte = rule->mnemonic == Mnemonic::kBfldh;
      instruction.operands.push_back(std::make_unique<BitWordOperand>(BitWordAddress(byte1)));
      instruction.operands.push_back(MakeImmediate(raw[high_byte ? 2 : 3]));
      instruction.operands.push_back(MakeImmediate(raw[high_byte ? 3 : 2]));
      return instruction;
    }

    case Format::kBitRel: {
      if (LowNibble(raw[3]) != 0) {
        Invalid(pc, raw);
      }
      instruction.operands.push_back(
          MakeBitOperand(byte1, static_cast<Byte>(raw[3] >> 4)));
      instruction.operands.push_back(
          MakeRelative(static_cast<std::int8_t>(raw[2])));
      return instruction;
    }

    case Format::kTrap:
      if ((byte1 & 1) != 0) Invalid(pc, raw);
      instruction.operands.push_back(MakeImmediate(byte1 >> 1));
      return instruction;

    case Format::kProtected:
      if (byte1 != static_cast<Byte>(~opcode) || raw[2] != opcode || raw[3] != opcode) {
        instruction.protection_fault = true;
      }
      return instruction;

    case Format::kReti:
      if (byte1 != 0x88) {
        Invalid(pc, raw);
      }
      return instruction;

    case Format::kExtAtomic: {
      const bool atomic = high <= 0x03;
      const bool extr = high >= 0x08 && high <= 0x0B;
      if (low != 0 || (!atomic && !extr)) {
        Invalid(pc, raw);
      }
      const Byte range = static_cast<Byte>((high & 0x03) + 1);
      instruction.mnemonic = extr ? Mnemonic::kExtr : Mnemonic::kAtomic;
      instruction.operands.push_back(MakeImmediate(range));
      return instruction;
    }

    case Format::kExtendedAddress: {
      // C166S V1, pp. 6-40–6-47. High nibble: R, P, range - 1.
      const bool page = (high & 0x04) != 0;
      const bool extended_registers = (high & 0x08) != 0;
      if (extended_registers) {
        instruction.mnemonic = page ? Mnemonic::kExtpr : Mnemonic::kExtsr;
      } else {
        instruction.mnemonic = page ? Mnemonic::kExtp : Mnemonic::kExts;
      }
      if (rule->size == 4) {
        if (low != 0 || word > (page ? 0x3FF : 0xFF)) Invalid(pc, raw);
        instruction.operands.push_back(MakeImmediate(word));
      } else {
        instruction.operands.push_back(MakeRegister(low));
      }
      instruction.operands.push_back(MakeImmediate((high & 0x03) + 1));
      return instruction;
    }

    case Format::kWordByteReg:
      instruction.operands.push_back(MakeRegister(low));
      instruction.operands.push_back(MakeByteRegister(high));
      return instruction;
  }

  Invalid(pc, raw);
}

}  // namespace c166
