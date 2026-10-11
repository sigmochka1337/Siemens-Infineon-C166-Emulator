#pragma once
#include <cstdint>
#include "src/includes/conditions.h"
#include "src/includes/types.h"

class Stack
{
    uint16_t SP;

    void push(Word word);
    uint16_t pop();

    void call(FuncAddress address);
    void calla(Condition cc, FuncAddress address);
    void calli(Condition cc, FuncAddress address);
    void callr(FuncOffset offset);
    void calls(SegNumber seg, FuncAddress address);
    void pcall(Word word, FuncAddress address);

    void ret();
    void rets();
    void retp(Word &word);

    void jmpa(Condition cc, FuncAddress address);
    void jmpi(Condition cc, FuncAddress address);
    void jmpr(Condition cc, FuncOffset offset);
    void jmps(SegNumber seg, FuncAddress address);
    void jb(BitAddress bit_address, FuncAddress address);
    void jnb(BitAddress bit_address, FuncAddress address);
    void jbc(BitAddress bit_address, FuncAddress address);
    void jnbc(BitAddress bit_address, FuncAddress address);
};