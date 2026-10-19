#pragma once

#include <cstdint>

// Предложение интерфейса памяти (ТЗ, п. 3.2.6).
// Владелец компонента Memory может менять реализацию, но сигнатуры должны
// остаться: Fetch и Execute зависят только от них.
namespace c166 {

struct MemoryFault {
    uint32_t address = 0;
};

class IMemory {
public:
    virtual ~IMemory() = default;

    // Адрес — уже физический 24-битный (0x000000..0xFFFFFF).
    // Перевод логического адреса через DPP делает сам компонент Memory.
    virtual uint8_t read8(uint32_t address) const = 0;
    virtual uint16_t read16(uint32_t address) const = 0;
    virtual void write8(uint32_t address, uint8_t value) = 0;
    virtual void write16(uint32_t address, uint16_t value) = 0;
};

} // namespace c166
