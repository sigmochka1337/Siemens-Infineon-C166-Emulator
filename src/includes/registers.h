#pragma once
#include <cstdint>
#include "src/includes/stack.h"
#include "src/includes/psw.h"

class Registers
{
    uint16_t R[16];
    Stack *stack = nullptr;
    uint16_t code_segment_pointer;
    uint16_t instruction_pointer;
    uint16_t dpp[4];
    uint16_t context_pointer;
    PSW *PSW;
    uint16_t MDH;
    uint16_t MDL;

    uint32_t MD();
};