// Тесты Fetch/Decode. Без внешних зависимостей: собираются g++/clang/MSVC.
#include <cstdio>
#include <initializer_list>
#include <string>
#include <vector>

#include "src/includes/decoder.h"

using namespace c166;

namespace {

int g_checks = 0;
int g_failures = 0;

void check(bool ok, const char* expr, int line) {
    ++g_checks;
    if (!ok) {
        ++g_failures;
        std::printf("  FAIL (line %d): %s\n", line, expr);
    }
}

#define CHECK(cond) check((cond), #cond, __LINE__)

class MockMemory : public IMemory {
public:
    explicit MockMemory(uint32_t size) : data_(size, 0) {}

    uint8_t read8(uint32_t address) const override {
        if (has_fault_ && address == fault_) {
            throw MemoryFault{address};
        }
        if (address >= data_.size()) {
            throw MemoryFault{address};
        }
        return data_[address];
    }

    uint16_t read16(uint32_t address) const override {
        return static_cast<uint16_t>(read8(address) |
                                     (static_cast<uint16_t>(read8(address + 1)) << 8));
    }

    void write8(uint32_t address, uint8_t value) override {
        if (address < data_.size()) {
            data_[address] = value;
        }
    }

    void write16(uint32_t address, uint16_t value) override {
        write8(address, static_cast<uint8_t>(value & 0xFF));
        write8(address + 1, static_cast<uint8_t>(value >> 8));
    }

    void set(uint32_t address, std::initializer_list<uint8_t> bytes) {
        uint32_t offset = 0;
        for (uint8_t byte : bytes) {
            data_[address + offset++] = byte;
        }
    }

    void faultAt(uint32_t address) {
        has_fault_ = true;
        fault_ = address;
    }

private:
    std::vector<uint8_t> data_;
    uint32_t fault_ = 0;
    bool has_fault_ = false;
};

// Таблица только для тестов: кодирование условное, проверяется механика.
DecodeTable testTable() {
    DecodeTable table{};
    table[0x10] = OpInfo{Op::ADD, Fmt::RegReg, 2};
    table[0x11] = OpInfo{Op::MOV, Fmt::RegImm16, 4};
    table[0x12] = OpInfo{Op::MOV, Fmt::RegMem, 4};
    table[0x13] = OpInfo{Op::JMPR, Fmt::Rel8, 2};
    table[0x14] = OpInfo{Op::NOP, Fmt::None, 2};
    table[0x15] = OpInfo{Op::ADD, Fmt::RegImm4, 2};
    return table;
}

void testFetch() {
    std::printf("fetch\n");

    MockMemory mem(0x20000);
    mem.set(0x100, {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88});

    const Fetched full = fetch(mem, 0x100);
    CHECK(full.valid == 8);
    CHECK(full.fault == false);
    CHECK(full.pc_start == 0x100);
    CHECK(full.bytes[0] == 0x11);
    CHECK(full.bytes[1] == 0x22);
    CHECK(full.bytes[7] == 0x88);

    // Окно не выходит за границу 64К-сегмента.
    const Fetched tail = fetch(mem, 0xFFFC);
    CHECK(tail.valid == 4);
    CHECK(tail.fault == false);

    // Ошибка на первом байте: инструкции нет.
    MockMemory broken(0x20000);
    broken.faultAt(0x200);
    const Fetched none = fetch(broken, 0x200);
    CHECK(none.valid == 0);
    CHECK(none.fault == true);

    // Ошибка после первого слова: слово есть, окно короткое.
    MockMemory partial(0x20000);
    partial.set(0x200, {0xAA, 0xBB});
    partial.faultAt(0x202);
    const Fetched two = fetch(partial, 0x200);
    CHECK(two.valid == 2);
    CHECK(two.fault == false);
    CHECK(two.bytes[0] == 0xAA);
    CHECK(two.bytes[1] == 0xBB);
}

void testDecodeErrors() {
    std::printf("decode: ошибки\n");

    MockMemory mem(0x1000);
    mem.set(0x100, {0x10, 0x21});

    DecodeError error;
    const Fetched fetched = fetch(mem, 0x100);

    const auto unknown = decode(fetched, emptyTable(), error);
    CHECK(!unknown.has_value());
    CHECK(error.kind == DecodeError::Kind::IllegalOpcode);
    CHECK(error.pc == 0x100);
    CHECK(error.opcode == 0x10);
    CHECK(std::string(error.what()) == "illegal opcode");

    // Инструкция на 4 байта, а прочитано 2.
    MockMemory short_mem(0x1000);
    short_mem.set(0x100, {0x11, 0x03});
    short_mem.faultAt(0x102);
    const Fetched truncated = fetch(short_mem, 0x100);
    CHECK(truncated.valid == 2);
    const auto result = decode(truncated, testTable(), error);
    CHECK(!result.has_value());
    CHECK(error.kind == DecodeError::Kind::Truncated);
    CHECK(std::string(error.what()) == "truncated instruction");
}

void testDecodeOperands() {
    std::printf("decode: операнды\n");

    MockMemory mem(0x1000);
    DecodeTable table = testTable();
    DecodeError error;

    // reg, reg
    mem.set(0x100, {0x10, 0x21});
    auto ins = decode(fetch(mem, 0x100), table, error);
    CHECK(ins.has_value());
    CHECK(ins->op == Op::ADD);
    CHECK(ins->length == 2);
    CHECK(ins->nops == 2);
    CHECK(ins->ops[0].kind == OpKind::Reg);
    CHECK(ins->ops[0].reg == 1);
    CHECK(ins->ops[1].reg == 2);
    CHECK(format(*ins) == "ADD R1, R2");

    // reg, #imm16
    mem.set(0x104, {0x11, 0x03, 0x34, 0x12});
    ins = decode(fetch(mem, 0x104), table, error);
    CHECK(ins.has_value());
    CHECK(ins->length == 4);
    CHECK(ins->ops[0].reg == 3);
    CHECK(ins->ops[1].kind == OpKind::Imm);
    CHECK(ins->ops[1].value == 0x1234);
    CHECK(format(*ins) == "MOV R3, #0x1234");

    // reg, mem
    mem.set(0x108, {0x12, 0x05, 0x78, 0x56});
    ins = decode(fetch(mem, 0x108), table, error);
    CHECK(ins.has_value());
    CHECK(ins->ops[0].reg == 5);
    CHECK(ins->ops[1].kind == OpKind::Mem);
    CHECK(ins->ops[1].value == 0x5678);
    CHECK(format(*ins) == "MOV R5, 0x5678");

    // rel8 со знаком
    mem.set(0x10C, {0x13, 0xF0});
    ins = decode(fetch(mem, 0x10C), table, error);
    CHECK(ins.has_value());
    CHECK(ins->nops == 1);
    CHECK(ins->ops[0].kind == OpKind::Rel);
    CHECK(ins->ops[0].rel == -16);
    CHECK(format(*ins) == "JMPR -16");

    // без операндов
    mem.set(0x110, {0x14, 0x00});
    ins = decode(fetch(mem, 0x110), table, error);
    CHECK(ins.has_value());
    CHECK(ins->nops == 0);
    CHECK(format(*ins) == "NOP");

    // imm4 в старшем ниббле второго байта
    mem.set(0x114, {0x15, 0x2A});
    ins = decode(fetch(mem, 0x114), table, error);
    CHECK(ins.has_value());
    CHECK(ins->ops[0].reg == 10);
    CHECK(ins->ops[1].value == 2);
    CHECK(format(*ins) == "ADD R10, #0x0002");

    // pc_start доезжает до инструкции
    CHECK(ins->pc_start == 0x114);

    // все мнемоники из списка имеют имя
    CHECK(std::string(opName(Op::UNKNOWN)) == "UNKNOWN");
    CHECK(std::string(opName(Op::JNBC)) == "JNBC");
    CHECK(std::string(opName(Op::DIVLU)) == "DIVLU");
}

} // namespace

int main() {
    testFetch();
    testDecodeErrors();
    testDecodeOperands();

    std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
