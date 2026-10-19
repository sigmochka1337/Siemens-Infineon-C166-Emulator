#pragma once
#include <cstdint>
#include <array>

struct CpuSnapshot {
    uint32_t pc = 0;
    uint16_t csp = 0;
    uint16_t ip = 0;
    std::array<uint16_t, 16> grp{};
    std::array<uint16_t, 4> dpp{};
    uint16_t sp = 0;
    uint32_t md = 0;
    bool psw_N = false;
    bool psw_C = false;
    bool psw_V = false;
    bool psw_Z = false;
};