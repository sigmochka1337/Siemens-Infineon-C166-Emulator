#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <string>

#include "src/includes/instruction.h"
#include "src/includes/memory_iface.h"

namespace c166 {

inline constexpr std::uint8_t kMaxInstructionBytes = 8;

struct Fetched {
  std::uint8_t bytes[kMaxInstructionBytes]{};
  std::uint8_t valid = 0;
  std::uint32_t pc_start = 0;
  bool fault = false;
};

enum class Fmt : std::uint8_t {
  kNone,
  kRegReg,
  kRegImm4,
  kRegImm8,
  kRegImm16,
  kRegMem,
  kMemReg,
  kRel8
};
struct OpInfo {
  Op op = Op::kUnknown;
  Fmt fmt = Fmt::kNone;
  std::uint8_t length = 2;
};
using DecodeTable = std::array<OpInfo, 256>;

const DecodeTable& EmptyTable();

struct DecodeError {
  enum class Kind : std::uint8_t {
    kNone,
    kIllegalOpcode,
    kTruncated
  } kind = Kind::kNone;
  std::uint32_t pc = 0;
  std::uint8_t opcode = 0;
  const char* What() const;
};

Fetched Fetch(const IMemory& memory, std::uint32_t address);
std::optional<Instruction> Decode(const Fetched& fetched,
                                  const DecodeTable& table, DecodeError& error);
std::optional<Instruction> DisassembleOne(const IMemory& memory,
                                          std::uint32_t address,
                                          const DecodeTable& table,
                                          DecodeError& error);
std::string Format(const Instruction& instruction);

}  // namespace c166
