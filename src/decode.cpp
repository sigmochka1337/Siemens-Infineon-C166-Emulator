#include "src/includes/decoder.h"

#include <cstdio>

namespace c166 {

namespace {

uint16_t read16(const Fetched& f, uint8_t offset) {
    return static_cast<uint16_t>(f.bytes[offset] | (static_cast<uint16_t>(f.bytes[offset + 1]) << 8));
}

uint8_t lowNibble(uint8_t byte) { return static_cast<uint8_t>(byte & 0x0Fu); }
uint8_t highNibble(uint8_t byte) { return static_cast<uint8_t>(byte >> 4); }

std::string formatOperand(const Operand& op) {
    char buffer[32] = {};
    switch (op.kind) {
    case OpKind::Reg:
        std::snprintf(buffer, sizeof buffer, "R%u", op.reg);
        break;
    case OpKind::RegByte:
        std::snprintf(buffer, sizeof buffer, "%s%u", op.high ? "RH" : "RL", op.reg);
        break;
    case OpKind::Imm:
        std::snprintf(buffer, sizeof buffer, "#0x%04X", op.value);
        break;
    case OpKind::Mem:
        std::snprintf(buffer, sizeof buffer, "0x%04X", op.value);
        break;
    case OpKind::Bit:
        std::snprintf(buffer, sizeof buffer, "bit 0x%04X", op.value);
        break;
    case OpKind::Rel:
        std::snprintf(buffer, sizeof buffer, "%+d", static_cast<int>(op.rel));
        break;
    case OpKind::None:
        break;
    }
    return buffer;
}

} // namespace

const char* opName(Op op) {
    switch (op) {
#define C166_AS_CASE(name) \
    case Op::name:         \
        return #name;
        C166_OP_LIST(C166_AS_CASE)
#undef C166_AS_CASE
    case Op::UNKNOWN:
        break;
    }
    return "UNKNOWN";
}

const char* DecodeError::what() const {
    switch (kind) {
    case Kind::None:
        return "none";
    case Kind::IllegalOpcode:
        return "illegal opcode";
    case Kind::Truncated:
        return "truncated instruction";
    }
    return "unknown error";
}

const DecodeTable& emptyTable() {
    static const DecodeTable table{};
    return table;
}

std::optional<Instruction> decode(const Fetched& fetched, const DecodeTable& table,
                                  DecodeError& error) {
    error = DecodeError{};
    error.pc = fetched.pc_start;

    if (fetched.valid < 2) {
        error.kind = DecodeError::Kind::Truncated;
        return std::nullopt;
    }

    const uint8_t opcode = fetched.bytes[0];
    error.opcode = opcode;

    const OpInfo& info = table[opcode];
    if (info.op == Op::UNKNOWN) {
        error.kind = DecodeError::Kind::IllegalOpcode;
        return std::nullopt;
    }
    if (fetched.valid < info.length) {
        error.kind = DecodeError::Kind::Truncated;
        return std::nullopt;
    }

    Instruction ins;
    ins.op = info.op;
    ins.length = info.length;
    ins.pc_start = fetched.pc_start;

    const uint8_t b1 = fetched.bytes[1];
    switch (info.fmt) {
    case Fmt::None:
        break;
    case Fmt::RegReg:
        ins.ops[0] = Operand{OpKind::Reg, lowNibble(b1), false, 0, 0};
        ins.ops[1] = Operand{OpKind::Reg, highNibble(b1), false, 0, 0};
        ins.nops = 2;
        break;
    case Fmt::RegImm4:
        ins.ops[0] = Operand{OpKind::Reg, lowNibble(b1), false, 0, 0};
        ins.ops[1] = Operand{OpKind::Imm, 0, false, highNibble(b1), 0};
        ins.nops = 2;
        break;
    case Fmt::RegImm8:
        ins.ops[0] = Operand{OpKind::Reg, lowNibble(b1), false, 0, 0};
        ins.ops[1] = Operand{OpKind::Imm, 0, false, highNibble(b1), 0};
        ins.nops = 2;
        break;
    case Fmt::RegImm16:
        ins.ops[0] = Operand{OpKind::Reg, lowNibble(b1), false, 0, 0};
        ins.ops[1] = Operand{OpKind::Imm, 0, false, read16(fetched, 2), 0};
        ins.nops = 2;
        break;
    case Fmt::RegMem:
        ins.ops[0] = Operand{OpKind::Reg, lowNibble(b1), false, 0, 0};
        ins.ops[1] = Operand{OpKind::Mem, 0, false, read16(fetched, 2), 0};
        ins.nops = 2;
        break;
    case Fmt::MemReg:
        ins.ops[0] = Operand{OpKind::Mem, 0, false, read16(fetched, 2), 0};
        ins.ops[1] = Operand{OpKind::Reg, lowNibble(b1), false, 0, 0};
        ins.nops = 2;
        break;
    case Fmt::Rel8:
        ins.ops[0] = Operand{OpKind::Rel, 0, false, 0, static_cast<int8_t>(b1)};
        ins.nops = 1;
        break;
    }

    return ins;
}

std::optional<Instruction> disassembleOne(const IMemory& mem, uint32_t address,
                                          const DecodeTable& table, DecodeError& error) {
    const Fetched fetched = fetch(mem, address);
    return decode(fetched, table, error);
}

std::string format(const Instruction& ins) {
    std::string text = opName(ins.op);
    for (uint8_t i = 0; i < ins.nops; ++i) {
        text += (i == 0) ? " " : ", ";
        text += formatOperand(ins.ops[i]);
    }
    return text;
}

} // namespace c166
