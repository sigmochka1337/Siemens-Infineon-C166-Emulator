#pragma once

#include <cstdint>

// Внутреннее представление инструкции C166.
// Контракт между Fetch/Decode и Execute/отладчиком.
namespace c166 {

// Перечень мнемоник v1 — по компонентам ТЗ (ALU, BitProcessor, Stack+Branch).
// UNKNOWN выставляется декодером, если опкод отсутствует в таблице.
#define C166_OP_LIST(X) \
    X(ADD) X(ADDB) X(ADDC) X(ADDCB) \
    X(SUB) X(SUBB) X(SUBC) X(SUBCB) \
    X(CMP) X(CMPB) X(NEG) X(NEGB) \
    X(MUL) X(MULU) X(DIV) X(DIVL) X(DIVU) X(DIVLU) \
    X(AND) X(ANDB) X(OR) X(ORB) X(XOR) X(XORB) X(CPL) X(CPLB) \
    X(SHL) X(SHR) X(ASHR) X(ROL) X(ROR) \
    X(MOV) X(MOVB) \
    X(PUSH) X(POP) \
    X(CALL) X(CALLA) X(CALLI) X(CALLR) X(CALLS) X(PCALL) \
    X(RET) X(RETS) X(RETP) \
    X(JMPA) X(JMPI) X(JMPR) X(JMPS) \
    X(JB) X(JNB) X(JBC) X(JNBC) \
    X(NOP) X(SRVWDT)

#define C166_AS_ENUM(name) name,

enum class Op : uint16_t {
    UNKNOWN = 0,
    C166_OP_LIST(C166_AS_ENUM)
};

#undef C166_AS_ENUM

const char* opName(Op op);

// Вид операнда. Декодер не ходит в память: он только описывает операнд,
// фактическое чтение/запись делает Execute через MemoryInterface.
enum class OpKind : uint8_t {
    None,
    Reg,     // R0..R15 (слово)
    RegByte, // RH0..RH7 / RL0..RL7
    Imm,     // непосредственное значение
    Mem,     // прямой адрес (в C166 резолвится через DPP в Memory)
    Bit,     // битовый адрес
    Rel      // относительное смещение со знаком
};

struct Operand {
    OpKind kind = OpKind::None;
    uint8_t reg = 0;
    bool high = false;
    uint16_t value = 0;
    int16_t rel = 0;
};

struct Instruction {
    Op op = Op::UNKNOWN;
    uint8_t length = 0;   // 2, 4, 6 или 8
    uint32_t pc_start = 0; // физический адрес инструкции (CSP:IP), для исключений
    uint8_t nops = 0;
    Operand ops[2];
};

} // namespace c166
