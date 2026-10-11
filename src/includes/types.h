#pragma once
#include <cstdint>

#define StrongType(Name, T)                         \
    class Name                                      \
    {                                               \
    public:                                         \
        explicit Name(const T &val) : value(val) {} \
        operator T() const { return value; }        \
                                                    \
    private:                                        \
        T value;                                    \
    };

StrongType(FuncAddress, uint16_t);
StrongType(FuncOffset, uint8_t);
StrongType(SegNumber, uint8_t);
StrongType(BitAddress, uint16_t);
StrongType(Word, uint16_t);
StrongType(Byte, uint8_t);
StrongType(ShiftCount, uint8_t);