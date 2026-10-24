#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

#include "decoder/decoder.h"
#include "decoder/fetcher.h"

namespace {
using namespace c166;

void Require(bool condition, const std::string& message) {
  if (!condition) {
    throw std::runtime_error(message);
  }
}

class CodeMemory final : public IMemoryBus {
 public:
  std::map<PhysicalAddress, Byte> bytes;
  std::vector<PhysicalAddress> reads;

  Result<Byte> Read8(PhysicalAddress address, AccessKind kind) override {
    Require(kind == AccessKind::kCode, "fetch must use code access");
    reads.push_back(address);
    return Peek8(address);
  }
  Result<Word> Read16(PhysicalAddress address, AccessKind kind) override {
    const auto low = Read8(address, kind);
    if (!low.Ok()) {
      return Result<Word>::Failure(low.Error());
    }
    const auto high = Read8(address + 1, kind);
    if (!high.Ok()) {
      return Result<Word>::Failure(high.Error());
    }
    return Result<Word>::Success(static_cast<Word>(low.Value() | (high.Value() << 8)));
  }
  Result<void> Write8(PhysicalAddress, Byte, AccessKind) override {
    throw std::runtime_error("fetch must not write memory");
  }
  Result<void> Write16(PhysicalAddress, Word, AccessKind) override {
    throw std::runtime_error("fetch must not write memory");
  }
  Result<Byte> Peek8(PhysicalAddress address) const override {
    const auto entry = bytes.find(address);
    if (entry == bytes.end()) {
      return Result<Byte>::Failure({FaultCode::kUnmapped, address, std::nullopt, "missing byte"});
    }
    return Result<Byte>::Success(entry->second);
  }
};

RawInstruction Raw(std::initializer_list<Byte> bytes) {
  RawInstruction raw;
  raw.pc = {0x12, 0x3456};
  raw.size = static_cast<Byte>(bytes.size());
  size_t index = 0;
  for (const auto byte : bytes) {
    raw.bytes[index++] = byte;
  }
  return raw;
}

void TestForms() {
  Decoder decoder;
  const auto nop = decoder.Decode(Raw({0xCC, 0x00}));
  Require(nop.Ok() && nop.Value().opcode == Opcode::kNop, "decode NOP");
  Require(nop.Value().operand_count == 0 && nop.Value().length == 2, "NOP shape");
  Require(nop.Value().pc == CodeAddress{0x12, 0x3456}, "preserve instruction PC");
  Require(decoder.Format(nop.Value()) == "NOP", "format NOP");

  for (Byte destination = 0; destination < 16; ++destination) {
    for (Byte source = 0; source < 16; ++source) {
      for (const Byte opcode : {Byte{0xF0}, Byte{0x00}}) {
        const Byte fields = static_cast<Byte>((destination << 4) | source);
        const auto decoded = decoder.Decode(Raw({opcode, fields}));
        Require(decoded.Ok(), "decode register operands");
        const auto& instruction = decoded.Value();
        Require(instruction.opcode == (opcode == 0xF0 ? Opcode::kMov : Opcode::kAdd),
                "register form opcode");
        Require(instruction.operand_count == 2, "register operand count");
        Require(std::get<RegisterOperand>(instruction.operands[0].value).id ==
                    static_cast<RegisterId>(destination),
                "high nibble is destination");
        Require(std::get<RegisterOperand>(instruction.operands[1].value).id ==
                    static_cast<RegisterId>(source),
                "low nibble is source");
      }
    }
    const auto immediate =
        decoder.Decode(Raw({0xE6, static_cast<Byte>(0xF0 | destination), 0x34, 0xAB}));
    Require(immediate.Ok() && immediate.Value().length == 4, "decode MOV immediate");
    Require(std::get<RegisterOperand>(immediate.Value().operands[0].value).id ==
                static_cast<RegisterId>(destination),
            "MOV short register F0..FF selects GPR");
    Require(std::get<ImmediateOperand>(immediate.Value().operands[1].value).value == 0xAB34,
            "MOV immediate little endian");
  }
  Require(decoder.Format(decoder.Decode(Raw({0xF0, 0xA3})).Value()) == "MOV R10, R3",
          "format word registers");
  Require(
      decoder.Format(decoder.Decode(Raw({0xE6, 0xF2, 0x34, 0xAB})).Value()) == "MOV R2, #0xAB34",
      "format immediate");

  for (Byte condition = 0; condition < 16; ++condition) {
    for (const Byte encoded : {Byte{0x00}, Byte{0x7F}, Byte{0x80}, Byte{0xFF}}) {
      const auto decoded =
          decoder.Decode(Raw({static_cast<Byte>((condition << 4) | 0x0D), encoded}));
      Require(decoded.Ok(), "decode all JMPR conditions");
      Require(decoded.Value().condition == static_cast<Condition>(condition), "condition encoding");
      const int expected = encoded < 128 ? encoded : encoded - 256;
      Require(std::get<RelativeOperand>(decoded.Value().operands[0].value).words == expected,
              "relative displacement is signed and measured in words");
    }
  }
  Require(decoder.Format(decoder.Decode(Raw({0x3D, 0xFF})).Value()) == "JMPR cc_NZ, -1",
          "format conditional backward jump");
}

void TestDecoderFailures() {
  Decoder decoder;
  const auto missing_first = decoder.Decode(Raw({0xCC}));
  Require(!missing_first.Ok() && missing_first.Error().code == FaultCode::kTruncatedInstruction,
          "missing first word");
  const auto missing_second = decoder.Decode(Raw({0xE6, 0xF0, 0x12}));
  Require(!missing_second.Ok() && missing_second.Error().code == FaultCode::kTruncatedInstruction,
          "missing second word");
  const auto extra_bytes = decoder.Decode(Raw({0xCC, 0x00, 0x00, 0x00}));
  Require(!extra_bytes.Ok() && extra_bytes.Error().code == FaultCode::kInvalidArgument,
          "exact raw instruction size required");
  const auto invalid_nop = decoder.Decode(Raw({0xCC, 0x01}));
  Require(!invalid_nop.Ok() && invalid_nop.Error().code == FaultCode::kInvalidEncoding,
          "reserved NOP field must be zero");
  const auto unsupported = decoder.Decode(Raw({0x01, 0x00}));
  Require(!unsupported.Ok() && unsupported.Error().code == FaultCode::kUnsupportedInstruction,
          "unimplemented encoding is not declared illegal");
  Require(decoder.InstructionLength({0xE6, 0x00}).Value() == 4,
          "known MOV family length independent of implemented destination subset");
  const auto unsupported_sfr = decoder.Decode(Raw({0xE6, 0x00, 0x01, 0x00}));
  Require(
      !unsupported_sfr.Ok() && unsupported_sfr.Error().code == FaultCode::kUnsupportedInstruction,
      "MOV immediate SFR form explicitly unsupported");
}

void TestFetchBoundaries() {
  Decoder decoder;
  Fetcher fetcher;
  CodeMemory memory;
  memory.bytes = {{0x12FFFE, 0xCC}, {0x12FFFF, 0x00}};
  const auto last_word = fetcher.Fetch({0x12, 0xFFFE}, memory, decoder);
  Require(last_word.Ok() && last_word.Value().size == 2, "two-byte fetch at segment end");
  Require(memory.reads.size() == 2, "do not speculatively read four or eight bytes");

  memory.reads.clear();
  const auto odd = fetcher.Fetch({0x12, 0xFFFF}, memory, decoder);
  Require(!odd.Ok() && odd.Error().trap == TrapKind::kIllegalInstructionAddress,
          "odd code address trap");
  Require(memory.reads.empty(), "odd PC rejected before reading");

  memory.bytes = {{0x12FFFC, 0xE6}, {0x12FFFD, 0xF0}, {0x12FFFE, 0x34}, {0x12FFFF, 0x12}};
  const auto last_instruction = fetcher.Fetch({0x12, 0xFFFC}, memory, decoder);
  Require(last_instruction.Ok() && last_instruction.Value().size == 4,
          "four-byte instruction fits at FFFC");
  Require(memory.reads.size() == 4, "fetch exactly four instruction bytes");
  Require(decoder.Decode(last_instruction.Value()).Ok(), "fetched bytes decode");

  memory.reads.clear();
  memory.bytes = {{0x12FFFE, 0xE6}, {0x12FFFF, 0xF0}, {0x130000, 0x00}, {0x130001, 0x00}};
  const auto crossing = fetcher.Fetch({0x12, 0xFFFE}, memory, decoder);
  Require(!crossing.Ok() && crossing.Error().code == FaultCode::kSegmentBoundary,
          "cross-segment fetch is a profile diagnostic");
  Require(!crossing.Error().trap.has_value() && memory.reads.size() == 2,
          "do not invent hardware trap or carry into CSP");

  memory.reads.clear();
  memory.bytes = {{0x000100, 0xE6}, {0x000101, 0xF0}};
  const auto missing_second = fetcher.Fetch({0, 0x0100}, memory, decoder);
  Require(!missing_second.Ok() && missing_second.Error().code == FaultCode::kUnmapped,
          "propagate second-word memory fault");
  Require(missing_second.Error().address == 0x000102, "preserve failing physical address");

  memory.reads.clear();
  memory.bytes = {{0x000100, 0xCC}};
  const auto missing_first = fetcher.Fetch({0, 0x0100}, memory, decoder);
  Require(!missing_first.Ok() && missing_first.Error().address == 0x000101,
          "preserve first-word memory fault");
}
}  // namespace

int main() {
  try {
    TestForms();
    TestDecoderFailures();
    TestFetchBoundaries();
  } catch (const std::exception& error) {
    std::cerr << "decoder/fetcher: " << error.what() << '\n';
    return 1;
  }
  std::cout << "decoder/fetcher tests passed\n";
  return 0;
}
