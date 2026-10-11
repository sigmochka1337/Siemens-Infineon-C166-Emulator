#pragma once
#include "src/includes/types.h"

namespace ALU
{
    void add(Word &op1, Word op2);
    void addb(Byte &op1, Byte op2);
    void addc(Word &op1, Word op2);
    void addcb(Byte &op1, Byte op2);

    void sub(Word &op1, Word op2);
    void subb(Byte &op1, Byte op2);
    void subc(Word &op1, Word op2);
    void subcb(Byte &op1, Byte op2);

    void mul(Word op1, Word op2);
    void mulu(Word op1, Word op2);

    void div(Word op1);
    void divl(Word op1);
    void divu(Word op1);
    void divlu(Word op1);

    void cmp(Word op1, Word op2);
    void cmpb(Byte op1, Byte op2);

    void neg(Word &op1);
    void negb(Byte &op1);
};