#pragma once
#include "src/includes/types.h"

namespace BitProcessor
{
    void and_(Word &op1, Word op2);
    void andb(Byte &op1, Byte op2);

    void or_(Word &op1, Word op2);
    void orb(Byte &op1, Byte op2);

    void xor_(Word &op1, Word op2);
    void xorb(Byte &op1, Byte op2);

    void cpl(Word &op1);
    void cplb(Byte &op1);

    void shl(Word &op1, ShiftCount op2);
    void shr(Word &op1, ShiftCount op2);
    void ashr(Word &op1, ShiftCount op2);
    void rol(Word &op1, ShiftCount op2);
    void ror(Word &op1, ShiftCount op2);
};