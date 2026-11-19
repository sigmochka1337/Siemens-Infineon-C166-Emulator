volatile unsigned short init_word = 0x1234u;
volatile unsigned short zero_word;
volatile unsigned short operand = 0xFFFFu;

volatile unsigned short startup_ok;
volatile unsigned short wrap16;
volatile unsigned long sum32;
volatile unsigned short words[4];
volatile unsigned short ram_sum;
volatile unsigned short call_result;
volatile unsigned short done;

unsigned short mix_word(unsigned short value)
{
    return (unsigned short)((value << 1) ^ 0x00FFu);
}

void main(void)
{
    unsigned short i;
    unsigned short value;
    unsigned short total = 0u;

    done = 0u;
    startup_ok = (init_word == 0x1234u && zero_word == 0u);

    value = operand;
    wrap16 = (unsigned short)(value + 2u);
    sum32 = (unsigned long)value + 2UL;

    for (i = 0u; i < 4u; ++i) {
        words[i] = (unsigned short)(0x1234u + i);
    }
    for (i = 0u; i < 4u; ++i) {
        total = (unsigned short)(total + words[i]);
    }
    ram_sum = total;

    call_result = mix_word(init_word);
    done = 0xA55Au;
    for (;;) { }
}