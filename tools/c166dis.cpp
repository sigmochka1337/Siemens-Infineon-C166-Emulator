// c166dis — демонстрация Fetch + Decode без процессора.
//
// Принимает байты, кладёт их в плоскую память по заданному адресу, прогоняет
// fetch → decode и печатает результат. Нужен, чтобы показать работу модуля
// до появления executor и загрузчика бинарников.
//
// ВАЖНО: таблица опкодов здесь демонстрационная. Она проверяет механику
// (форматы операндов, длину, ошибки), но НЕ сверена с Instruction Set Manual,
// поэтому в выводе всегда печатается предупреждение.

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#include "src/includes/decoder.h"

using namespace c166;

namespace {

constexpr uint32_t kMemorySize = 0x1'0000; // 64 КБ, один сегмент

class FlatMemory : public IMemory {
public:
    FlatMemory() : data_(kMemorySize, 0) {}

    uint8_t read8(uint32_t address) const override {
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
        if (address >= data_.size()) {
            throw MemoryFault{address};
        }
        data_[address] = value;
    }

    void write16(uint32_t address, uint16_t value) override {
        write8(address, static_cast<uint8_t>(value & 0xFF));
        write8(address + 1, static_cast<uint8_t>(value >> 8));
    }

private:
    std::vector<uint8_t> data_;
};

// Демонстрационная таблица: покрывает все форматы операндов, но коды условные.
DecodeTable demoTable() {
    DecodeTable table{};
    table[0x10] = OpInfo{Op::ADD, Fmt::RegReg, 2};
    table[0x11] = OpInfo{Op::MOV, Fmt::RegImm16, 4};
    table[0x12] = OpInfo{Op::MOV, Fmt::RegMem, 4};
    table[0x13] = OpInfo{Op::JMPR, Fmt::Rel8, 2};
    table[0x14] = OpInfo{Op::NOP, Fmt::None, 2};
    table[0x15] = OpInfo{Op::ADD, Fmt::RegImm4, 2};
    return table;
}

void usage(const char* program) {
    std::printf(
        "c166dis — разбор инструкции C166 из байтов (демо Fetch + Decode)\n"
        "\n"
        "Использование:\n"
        "  %s [-a АДРЕС] БАЙТЫ...\n"
        "\n"
        "  БАЙТЫ   hex, можно слитно или через пробел: '11033412' или '11 03 34 12'\n"
        "  -a      адрес в памяти, по умолчанию 0x0100\n"
        "\n"
        "Примеры:\n"
        "  %s 11 03 34 12      -> MOV R3, #0x1234\n"
        "  %s 10 21            -> ADD R1, R2\n"
        "  %s FF FF            -> ошибка: illegal opcode\n",
        program, program, program, program);
}

bool parseHexBytes(const std::string& text, std::vector<uint8_t>& out) {
    std::string digits;
    for (char c : text) {
        const auto uc = static_cast<unsigned char>(c);
        if (c == ' ' || c == ',' || c == '\t' || c == '_') {
            continue;
        }
        if (!std::isxdigit(uc)) {
            return false;
        }
        digits += c;
    }
    if (digits.empty() || digits.size() % 2 != 0) {
        return false;
    }
    for (size_t i = 0; i < digits.size(); i += 2) {
        out.push_back(static_cast<uint8_t>(std::stoi(digits.substr(i, 2), nullptr, 16)));
    }
    return true;
}

} // namespace

int main(int argc, char** argv) {
    uint32_t address = 0x0100;
    std::string hex;

    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            usage(argv[0]);
            return 0;
        }
        if (arg == "-a" || arg == "--addr") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "error: -a требует адрес\n");
                return 2;
            }
            address = static_cast<uint32_t>(std::strtoul(argv[++i], nullptr, 0));
            continue;
        }
        hex += arg;
    }

    std::vector<uint8_t> bytes;
    if (!parseHexBytes(hex, bytes)) {
        std::fprintf(stderr, "error: ожидались hex-байты, например '11 03 34 12'\n");
        return 2;
    }
    if (bytes.size() > kMemorySize || address + bytes.size() > kMemorySize) {
        std::fprintf(stderr, "error: байты не помещаются в демо-память (64 КБ)\n");
        return 2;
    }

    FlatMemory memory;
    for (size_t i = 0; i < bytes.size(); ++i) {
        memory.write8(address + static_cast<uint32_t>(i), bytes[i]);
    }

    std::fprintf(stderr, "warning: таблица опкодов демонстрационная, с мануалом не сверена\n");

    DecodeError error;
    const auto ins = disassembleOne(memory, address, demoTable(), error);

    std::printf("addr 0x%06X  bytes", address);
    for (size_t i = 0; i < bytes.size(); ++i) {
        std::printf(" %02X", bytes[i]);
    }
    std::printf("\n");

    if (!ins) {
        std::printf("error: %s (opcode 0x%02X) at 0x%06X\n", error.what(), error.opcode, error.pc);
        return 1;
    }

    std::printf("len  %u\n", ins->length);
    std::printf("asm  %s\n", format(*ins).c_str());
    return 0;
}
