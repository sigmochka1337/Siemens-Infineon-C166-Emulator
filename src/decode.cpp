#include <sstream>

#include "src/includes/decoder.h"

namespace c166 {

const DecodeTable& EmptyTable() {
  static const DecodeTable table{};
  return table;
}

const char* DecodeError::What() const {
  return kind == Kind::kIllegalOpcode ? "illegal opcode"
         : kind == Kind::kTruncated   ? "truncated instruction"
                                      : "";
}

const char* OpName(Op op) {
  switch (op) {
    case Op::kNop:
      return "NOP";
    case Op::kMov:
      return "MOV";
    case Op::kAdd:
      return "ADD";
    case Op::kSub:
      return "SUB";
    case Op::kPush:
      return "PUSH";
    case Op::kPop:
      return "POP";
    case Op::kCall:
      return "CALL";
    case Op::kRet:
      return "RET";
    case Op::kJmp:
      return "JMP";
    default:
      return "UNKNOWN";
  }
}

std::optional<Instruction> Decode(const Fetched& fetched,
                                  const DecodeTable& table,
                                  DecodeError& error) {
  error = {};
  error.pc = fetched.pc_start;
  if (fetched.valid == 0 || fetched.fault) {
    error.kind = DecodeError::Kind::kTruncated;
    return std::nullopt;
  }
  error.opcode = fetched.bytes[0];
  const OpInfo info = table[fetched.bytes[0]];
  if (info.op == Op::kUnknown) {
    error.kind = DecodeError::Kind::kIllegalOpcode;
    return std::nullopt;
  }
  if (fetched.valid < info.length) {
    error.kind = DecodeError::Kind::kTruncated;
    return std::nullopt;
  }
  Instruction instruction;
  instruction.op = info.op;
  instruction.length = info.length;
  instruction.pc_start = fetched.pc_start;
  return instruction;
}

std::optional<Instruction> DisassembleOne(const IMemory& memory,
                                          std::uint32_t address,
                                          const DecodeTable& table,
                                          DecodeError& error) {
  return Decode(Fetch(memory, address), table, error);
}

std::string Format(const Instruction& instruction) {
  std::ostringstream out;
  out << OpName(instruction.op);
  return out.str();
}

}  // namespace c166
