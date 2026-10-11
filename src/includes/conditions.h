#pragma once

enum Condition
{
    UC,  // always true
    Z,   // PSW.Z == 1
    NZ,  // PSW.Z == 0
    V,   // PSW.V == 1
    NV,  // PSW.V == 0
    N,   // PSW.N == 1
    NN,  // PSW.N == 0
    C,   // PSW.C == 1
    NC,  // PSW.C == 0
    EQ,  // cmp R0, R1  when R0 == R1
    NE,  // cmp R0, R1  when R0 != R1
    ULT, // cmp R0, R1 when R0 < R1   (unsigned)
    ULE, // cmp R0, R1 when R0 <= R1  (unsigned)
    UGE, // cmp R0, R1 when R0 >= R1  (unsigned)
    UGT, // cmp R0, R1 when R0 > R1   (unsigned)
    SLT, // cmp R0, R1 when R0 < R1   (signed)
    SLE, // cmp R0, R1 when R0 <= R1  (signed)
    SGE, // cmp R0, R1 when R0 >= R1  (signed)
    SGT, // cmp R0, R1 when R0 > R1   (signed)
};