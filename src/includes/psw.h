#pragma once

class PSW
{
    bool negative;
    bool carry;
    bool overflow;
    bool zero;
    bool usr0;

    bool &N = negative;
    bool &C = carry;
    bool &V = overflow;
    bool &Z = zero;
};