#pragma once

#include <array>
#include <optional>
#include <string>

#include "src/includes/instruction.h"
#include "src/includes/memory_iface.h"

namespace c166 {

inline constexpr uint8_t kMaxInstructionBytes = 8;

// Результат выборки: окно до 8 байт, начиная с pc_start.
// Длину инструкции определяет декодер, а не fetch: длина зависит от опкода
// и формата операндов, и дублировать эту таблицу в fetch незачем.
struct Fetched {
    uint8_t bytes[kMaxInstructionBytes] = {};
    uint8_t valid = 0;    // сколько байт реально прочитано
    uint32_t pc_start = 0;
    bool fault = false;   // не прочиталось даже первое слово
};

// Формат операндов. Конкретная раскладка полей берётся из таблицы опкодов
// (Instruction Set Manual), а не зашита в код.
enum class Fmt : uint8_t {
    None,     // без операндов
    RegReg,   // dest = младший ниббл, src = старший ниббл
    RegImm4,  // reg = младший ниббл, imm = старший ниббл
    RegImm8,  // reg = байт 1 (младший ниббл), imm = байт 1 (старший ниббл)
    RegImm16, // reg = младший ниббл байта 1, imm = слово по смещению 2
    RegMem,   // reg = младший ниббл байта 1, mem = слово по смещению 2
    MemReg,   // mem = слово по смещению 2, reg = младший ниббл байта 1
    Rel8      // rel = байт 1 со знаком
};

struct OpInfo {
    Op op = Op::UNKNOWN;
    Fmt fmt = Fmt::None;
    uint8_t length = 2;
};

using DecodeTable = std::array<OpInfo, 256>;

// Таблица-заглушка: все опкоды UNKNOWN. Боевая таблица заполняется из
// Instruction Set Manual (см. docs/isa_scope.md) и генерируется из одного
// источника, чтобы декодер и дизассемблер не разъезжались.
const DecodeTable& emptyTable();

struct DecodeError {
    enum class Kind : uint8_t { None, IllegalOpcode, Truncated } kind = Kind::None;
    uint32_t pc = 0;
    uint8_t opcode = 0;
    const char* what() const;
};

// Чистая функция: не меняет состояние процессора и не обращается к памяти.
Fetched fetch(const IMemory& mem, uint32_t address);

// Чистая функция: только разбор уже прочитанных байт.
std::optional<Instruction> decode(const Fetched& fetched, const DecodeTable& table,
                                  DecodeError& error);

// Текстовое представление для отладчика (ТЗ, «Обязательно», п. 7).
std::string format(const Instruction& ins);

} // namespace c166
