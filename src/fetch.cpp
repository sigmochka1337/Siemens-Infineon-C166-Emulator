#include "src/includes/decoder.h"

#include <algorithm>

namespace c166 {

namespace {

constexpr uint32_t kAddressSpace = 0x0100'0000u; // 24 бита
constexpr uint32_t kSegmentSize = 0x0001'0000u;  // 64 КБ

} // namespace

Fetched fetch(const IMemory& mem, uint32_t address) {
    Fetched out;
    out.pc_start = address;

    // Выборка не выходит за границу текущего сегмента: переход через границу
    // (перенос в CSP) — ответственность цикла исполнения, а не fetch.
    const uint32_t segment_end = (address & ~(kSegmentSize - 1)) + kSegmentSize;
    const uint32_t limit = std::min(segment_end, kAddressSpace);

    for (uint8_t i = 0; i < kMaxInstructionBytes; ++i) {
        if (address + i >= limit) {
            break;
        }
        try {
            out.bytes[i] = mem.read8(address + i);
            out.valid = static_cast<uint8_t>(i + 1);
        } catch (const MemoryFault&) {
            break; // остаток окна недостоверен
        }
    }

    // Минимальная инструкция C166 — слово: меньше двух байт декодировать нечего.
    out.fault = out.valid < 2;
    return out;
}

} // namespace c166
