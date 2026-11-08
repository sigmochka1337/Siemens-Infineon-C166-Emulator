// Golden-векторы Fetch/Decode. Байты взяты из реального вывода Keil A166
// V5.38.2.0 (C166 classic) и сверены с описанием операндов C166 ISM V2.0.
#include <cstdio>
#include <exception>
#include <initializer_list>
#include <string>
#include <vector>

#include "exceptions.h"
#include "fetcher.h"
#include "flat_machine.h"
#include "loader.h"

namespace {

using namespace c166;

int g_checks = 0;
int g_failures = 0;

void Check(bool condition, const char* expression, int line) {
  ++g_checks;
  if (!condition) {
    ++g_failures;
    std::printf("  FAIL (line %d): %s\n", line, expression);
  }
}

#define CHECK(condition) Check((condition), #condition, __LINE__)

constexpr PhysicalAddress kBase = 0x100;

void Put(FlatMachine& machine, PhysicalAddress address,
         std::initializer_list<Byte> bytes) {
  PhysicalAddress current = address;
  for (Byte byte : bytes) {
    machine.Write8(current, byte);
    ++current;
  }
}

std::string FetchText(FlatMachine& machine, std::initializer_list<Byte> bytes,
                      Byte* size = nullptr) {
  machine.ClearMemory();
  Put(machine, kBase, bytes);
  Fetcher fetcher(machine);
  const Instruction instruction = fetcher.Fetch(CodeAddress{0, kBase});
  if (size != nullptr) {
    *size = instruction.size;
  }
  return instruction.ToString();
}

template <typename Callable>
bool ThrowsUnreadable(Callable&& callable) {
  try {
    callable();
  } catch (const UnreadableInstruction&) {
    return true;
  } catch (...) {
    return false;
  }
  return false;
}

void TestGoldenVectors() {
  std::printf("fetcher: golden vectors\n");

  FlatMachine machine(0x20000);

  struct Case {
    std::initializer_list<Byte> bytes;
    Byte size;
    const char* text;
  };

  const Case cases[] = {
      // Управление, стек, системные.
      {{0xCC, 0x00}, 2, "nop"},
      {{0xCB, 0x00}, 2, "ret"},
      {{0xDB, 0x00}, 2, "rets"},
      {{0xFB, 0x88}, 2, "reti"},
      {{0xEC, 0xF3}, 2, "push R3"},
      {{0xFC, 0xF3}, 2, "pop R3"},
      {{0xEC, 0x88}, 2, "push PSW"},
      {{0xFC, 0x88}, 2, "pop PSW"},
      {{0xEC, 0x09}, 2, "push SP"},
      {{0xEC, 0x08}, 2, "push CP"},
      {{0xEC, 0x00}, 2, "push DPP0"},
      {{0xEC, 0x07}, 2, "push MDL"},
      {{0xEC, 0x06}, 2, "push MDH"},
      {{0xEC, 0x87}, 2, "push MDC"},
      {{0xEC, 0x0A}, 2, "push STKOV"},
      {{0xEC, 0x0B}, 2, "push STKUN"},
      {{0xEB, 0xF3}, 2, "retp R3"},
      {{0xEB, 0x88}, 2, "retp PSW"},
      {{0xEB, 0x09}, 2, "retp SP"},
      {{0x9B, 0x02}, 2, "trap #0x0001"},
      {{0xA7, 0x58, 0xA7, 0xA7}, 4, "srvwdt"},
      {{0xD1, 0x80}, 2, "extr #0x0001"},
      {{0xD1, 0x90}, 2, "extr #0x0002"},
      {{0xD1, 0xA0}, 2, "extr #0x0003"},
      {{0xD1, 0x00}, 2, "atomic #0x0001"},
      {{0xD1, 0x10}, 2, "atomic #0x0002"},

      // Ветвление.
      {{0xBB, 0x17}, 2, "callr +23"},
      {{0xDA, 0x01, 0x34, 0x12}, 4, "calls 0x01:0x1234"},
      {{0xFA, 0x03, 0x34, 0x12}, 4, "jmps 0x03:0x1234"},
      {{0xCA, 0x20, 0x34, 0x12}, 4, "calla cc_Z, 0x1234"},
      {{0xEA, 0x80, 0x34, 0x12}, 4, "jmpa cc_C, 0x1234"},
      {{0xAB, 0x22}, 2, "calli cc_Z, [R2]"},
      {{0x9C, 0x94}, 2, "jmpi cc_NC, [R4]"},
      {{0xE2, 0xF0, 0x34, 0x12}, 4, "pcall R0, 0x1234"},
      {{0xE2, 0x88, 0x34, 0x12}, 4, "pcall PSW, 0x1234"},

      // JB/JNB/JBC/JNBS + BSET/BCLR (P3.10 = 0xFFC4.10).
      {{0x8A, 0xE2, 0x06, 0xA0}, 4, "jb 0xFFC4.10, +6"},
      {{0x9A, 0xE2, 0x05, 0xA0}, 4, "jnb 0xFFC4.10, +5"},
      {{0xAA, 0xE2, 0x04, 0xA0}, 4, "jbc 0xFFC4.10, +4"},
      {{0xBA, 0xE2, 0x03, 0xA0}, 4, "jnbs 0xFFC4.10, +3"},
      {{0xAF, 0xE2}, 2, "bset 0xFFC4.10"},
      {{0xAE, 0xE2}, 2, "bclr 0xFFC4.10"},
      {{0x0F, 0xE2}, 2, "bset 0xFFC4.0"},
      {{0x0F, 0x00}, 2, "bset 0xFD00.0"},
      {{0x0E, 0xE2}, 2, "bclr 0xFFC4.0"},
      {{0x8A, 0xE2, 0x06, 0x00}, 4, "jb 0xFFC4.0, +6"},

      // ALU, word.
      {{0x00, 0x01}, 2, "add R0, R1"},
      {{0x10, 0x01}, 2, "addc R0, R1"},
      {{0x20, 0x01}, 2, "sub R0, R1"},
      {{0x30, 0x01}, 2, "subc R0, R1"},
      {{0x40, 0x01}, 2, "cmp R0, R1"},
      {{0x08, 0x01}, 2, "add R0, #0x0001"},
      {{0x48, 0x03}, 2, "cmp R0, #0x0003"},
      {{0x46, 0xF0, 0x34, 0x12}, 4, "cmp R0, #0x1234"},
      {{0x48, 0x09}, 2, "cmp R0, [R1]"},
      {{0x48, 0x0D}, 2, "cmp R0, [R1+]"},
      {{0x42, 0xF0, 0x34, 0x12}, 4, "cmp R0, 0x1234"},
      {{0x08, 0x1E}, 2, "add R1, [R2+]"},
      {{0x18, 0x09}, 2, "addc R0, [R1]"},
      {{0x14, 0xF0, 0x34, 0x12}, 4, "addc 0x1234, R0"},
      {{0x32, 0xF0, 0x34, 0x12}, 4, "subc R0, 0x1234"},
      {{0x06, 0xF0, 0x34, 0x12}, 4, "add R0, #0x1234"},
      {{0x02, 0xF0, 0x34, 0x12}, 4, "add R0, 0x1234"},
      {{0x04, 0xF0, 0x34, 0x12}, 4, "add 0x1234, R0"},
      {{0x76, 0xE2, 0x00, 0x04}, 4, "or 0xFFC4, #0x0400"},
      {{0x66, 0xE3, 0xFF, 0xF7}, 4, "and 0xFFC6, #0xF7FF"},
      {{0x06, 0xE2, 0x01, 0x00}, 4, "add 0xFFC4, #0x0001"},
      {{0x74, 0xF0, 0xC4, 0xFF}, 4, "or 0xFFC4, R0"},
      {{0x72, 0xF0, 0xC4, 0xFF}, 4, "or R0, 0xFFC4"},
      {{0x81, 0x30}, 2, "neg R3"},
      {{0x0B, 0x34}, 2, "mul R3, R4"},
      {{0x1B, 0x34}, 2, "mulu R3, R4"},
      {{0x4B, 0x33}, 2, "div R3"},
      {{0x6B, 0x33}, 2, "divl R3"},
      {{0x5B, 0x33}, 2, "divu R3"},
      {{0x7B, 0x33}, 2, "divlu R3"},

      // ALU, byte.
      {{0x01, 0x02}, 2, "addb RL0, RL1"},
      {{0x11, 0x02}, 2, "addcb RL0, RL1"},
      {{0x21, 0x02}, 2, "subb RL0, RL1"},
      {{0x31, 0x02}, 2, "subcb RL0, RL1"},
      {{0x41, 0x02}, 2, "cmpb RL0, RL1"},
      {{0x09, 0x03}, 2, "addb RL0, #0x0003"},
      {{0x49, 0x03}, 2, "cmpb RL0, #0x0003"},
      {{0x07, 0xF0, 0x12, 0x00}, 4, "addb RL0, #0x0012"},
      {{0x09, 0x09}, 2, "addb RL0, [R1]"},
      {{0x09, 0x0D}, 2, "addb RL0, [R1+]"},
      {{0x03, 0xF0, 0x34, 0x12}, 4, "addb RL0, 0x1234"},
      {{0x05, 0xF0, 0x34, 0x12}, 4, "addb 0x1234, RL0"},
      {{0x07, 0xE2, 0x01, 0x00}, 4, "addb 0xFFC4, #0x0001"},
      {{0x43, 0xF0, 0x34, 0x12}, 4, "cmpb RL0, 0x1234"},
      {{0xA1, 0x60}, 2, "negb RL3"},

      // BitProcessor.
      {{0x60, 0x01}, 2, "and R0, R1"},
      {{0x70, 0x01}, 2, "or R0, R1"},
      {{0x50, 0x01}, 2, "xor R0, R1"},
      {{0x64, 0xF0, 0x34, 0x12}, 4, "and 0x1234, R0"},
      {{0x74, 0xF0, 0x34, 0x12}, 4, "or 0x1234, R0"},
      {{0x68, 0x0E}, 2, "and R0, [R2+]"},
      {{0x78, 0x03}, 2, "or R0, #0x0003"},
      {{0x91, 0x30}, 2, "cpl R3"},
      {{0x61, 0x02}, 2, "andb RL0, RL1"},
      {{0x71, 0x02}, 2, "orb RL0, RL1"},
      {{0x51, 0x02}, 2, "xorb RL0, RL1"},
      {{0xB1, 0x60}, 2, "cplb RL3"},
      {{0x5C, 0x83}, 2, "shl R3, #0x0008"},
      {{0x5C, 0xF0}, 2, "shl R0, #0x000F"},
      {{0x0C, 0x01}, 2, "rol R0, R1"},
      {{0x2C, 0x01}, 2, "ror R0, R1"},
      {{0x4C, 0x01}, 2, "shl R0, R1"},
      {{0x6C, 0x01}, 2, "shr R0, R1"},
      {{0xAC, 0x01}, 2, "ashr R0, R1"},
      {{0x7C, 0x30}, 2, "shr R0, #0x0003"},
      {{0xBC, 0x30}, 2, "ashr R0, #0x0003"},
      {{0x1C, 0x30}, 2, "rol R0, #0x0003"},
      {{0x3C, 0x30}, 2, "ror R0, #0x0003"},

      // MOV.
      {{0xE0, 0x10}, 2, "mov R0, #0x0001"},
      {{0xE0, 0x21}, 2, "mov R1, #0x0002"},
      {{0xE0, 0xF0}, 2, "mov R0, #0x000F"},
      {{0xE0, 0x0F}, 2, "mov R15, #0x0000"},
      {{0xE6, 0xF0, 0x34, 0x12}, 4, "mov R0, #0x1234"},
      {{0xF0, 0x01}, 2, "mov R0, R1"},
      {{0xA8, 0x01}, 2, "mov R0, [R1]"},
      {{0x98, 0x01}, 2, "mov R0, [R1+]"},
      {{0xB8, 0x01}, 2, "mov [R1], R0"},
      {{0xD4, 0x08, 0x02, 0x00}, 4, "mov R0, [R8 + #0x000002]"},
      {{0xC4, 0x01, 0x02, 0x00}, 4, "mov [R1 + #0x000002], R0"},
      {{0xF2, 0xF0, 0x34, 0x12}, 4, "mov R0, 0x1234"},
      {{0xF6, 0xF0, 0x34, 0x12}, 4, "mov 0x1234, R0"},
      {{0xE6, 0xE2, 0x34, 0x12}, 4, "mov 0xFFC4, #0x1234"},
      {{0xE6, 0x5A, 0x40, 0x00}, 4, "mov 0xFEB4, #0x0040"},
      {{0xF2, 0xF0, 0xC4, 0xFF}, 4, "mov R0, 0xFFC4"},
      {{0xF6, 0xF0, 0xC4, 0xFF}, 4, "mov 0xFFC4, R0"},
      {{0x84, 0x01, 0x34, 0x12}, 4, "mov [R1], 0x1234"},
      {{0x94, 0x01, 0x34, 0x12}, 4, "mov 0x1234, [R1]"},
      {{0x88, 0x01}, 2, "mov [-R1], R0"},
      {{0xC8, 0x12}, 2, "mov [R1], [R2]"},
      {{0xD8, 0x12}, 2, "mov [R1+], [R2]"},

      // MOVB.
      {{0xE1, 0x30}, 2, "movb RL0, #0x0003"},
      {{0xE7, 0xF0, 0x12, 0x00}, 4, "movb RL0, #0x0012"},
      {{0xF1, 0x02}, 2, "movb RL0, RL1"},
      {{0xF1, 0x6B}, 2, "movb RL3, RH5"},
      {{0xF1, 0x7A}, 2, "movb RH3, RL5"},
      {{0xA9, 0x01}, 2, "movb RL0, [R1]"},
      {{0x99, 0x01}, 2, "movb RL0, [R1+]"},
      {{0xB9, 0x01}, 2, "movb [R1], RL0"},
      {{0xF4, 0x01, 0x12, 0x00}, 4, "movb RL0, [R1 + #0x000012]"},
      {{0xE4, 0x01, 0x12, 0x00}, 4, "movb [R1 + #0x000012], RL0"},
      {{0xF3, 0xF0, 0x34, 0x12}, 4, "movb RL0, 0x1234"},
      {{0xF7, 0xF0, 0x34, 0x12}, 4, "movb 0x1234, RL0"},
      {{0xE7, 0xE2, 0x12, 0x00}, 4, "movb 0xFFC4, #0x0012"},
      {{0xF3, 0xF0, 0xC4, 0xFF}, 4, "movb RL0, 0xFFC4"},
      {{0xF7, 0xF0, 0xC4, 0xFF}, 4, "movb 0xFFC4, RL0"},
      {{0x89, 0x01}, 2, "movb [-R1], RL0"},
      {{0xC9, 0x12}, 2, "movb [R1], [R2]"},
      {{0xD9, 0x12}, 2, "movb [R1+], [R2]"},

      // MOVBS/MOVBZ.
      {{0xD0, 0x20}, 2, "movbs R0, RL1"},
      {{0xC0, 0x20}, 2, "movbz R0, RL1"},
      {{0xD2, 0xF0, 0x34, 0x12}, 4, "movbs R0, 0x1234"},
      {{0xC2, 0xF0, 0x34, 0x12}, 4, "movbz R0, 0x1234"},
      {{0xC5, 0xF0, 0x34, 0x12}, 4, "movbz 0x1234, RL0"},
      {{0xD5, 0xF0, 0x34, 0x12}, 4, "movbs 0x1234, RL0"},
  };

  for (const Case& test : cases) {
    Byte size = 0;
    const std::string text = FetchText(machine, test.bytes, &size);
    if (text != test.text || size != test.size) {
      std::printf("  case failed: expected '%s' (%u bytes), got '%s' (%u bytes)\n",
                  test.text, test.size, text.c_str(), size);
    }
    CHECK(text == test.text);
    CHECK(size == test.size);
  }
}

void TestConditions() {
  std::printf("fetcher: condition codes\n");

  FlatMachine machine(0x20000);
  const char* names[] = {"cc_UC",  "cc_NET", "cc_Z",  "cc_NZ",
                         "cc_V",   "cc_NV",  "cc_N",  "cc_NN",
                         "cc_C",   "cc_NC",  "cc_SGT", "cc_SLE",
                         "cc_SLT", "cc_SGE", "cc_UGT", "cc_ULE"};

  for (int condition = 0; condition < 16; ++condition) {
    const Byte opcode = static_cast<Byte>((condition << 4) | 0x0D);
    const std::string text = FetchText(machine, {opcode, 0x00});
    const std::string expected =
        std::string("jmpr ") + names[condition] + ", +0";
    if (text != expected) {
      std::printf("  condition %d: expected '%s', got '%s'\n", condition,
                  expected.c_str(), text.c_str());
    }
    CHECK(text == expected);
  }
}

void TestErrors() {
  std::printf("fetcher: errors\n");

  FlatMachine machine(0x20000);
  Fetcher fetcher(machine);

  machine.ClearMemory();
  Put(machine, kBase, {0xF8, 0x00});  // нет такой инструкции
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  machine.ClearMemory();
  Put(machine, kBase, {0xF2, 0x00, 0x34, 0x12});  // MOV Rw,mem без 0xF в byte1
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  machine.ClearMemory();
  Put(machine, kBase, {0xCC, 0x01});  // NOP с ненулевым вторым байтом
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  machine.ClearMemory();
  Put(machine, kBase, {0x81, 0x01});  // NEG с младшим нибблом != 0
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  machine.ClearMemory();
  Put(machine, kBase, {0x4B, 0x34});  // DIV: reg в обоих нибблах должен совпадать
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  machine.ClearMemory();
  Put(machine, kBase, {0xEB, 0x04});  // RETP: неизвестный special-регистр
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  machine.ClearMemory();
  Put(machine, kBase, {0xCA, 0x01, 0x34, 0x12});  // CALLA с мусором в byte1
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  machine.ClearMemory();
  Put(machine, kBase, {0xA7, 0x00, 0x00, 0x00});  // SRVWDT fixed bytes
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  machine.ClearMemory();
  Put(machine, kBase, {0xFB, 0x00});  // RETI fixed byte
  CHECK(ThrowsUnreadable([&] { (void)fetcher.Fetch(CodeAddress{0, kBase}); }));

  // Инструкция на 4 байта не должна пересекать границу 64-КБ сегмента.
  machine.ClearMemory();
  Put(machine, 0xFFFF, {0xE6, 0xF0});
  CHECK(ThrowsUnreadable(
      [&] { (void)fetcher.Fetch(CodeAddress{0, 0xFFFF}); }));

  // Fetch не меняет PC.
  machine.ClearMemory();
  Put(machine, kBase, {0xCC, 0x00});
  machine.SetPC(CodeAddress{0, 0x200});
  (void)fetcher.Fetch(CodeAddress{0, kBase});
  CHECK(machine.GetPC() == (CodeAddress{0, 0x200}));
}

void TestLoader() {
  std::printf("loader: Intel HEX и raw\n");

  const char* hex =
      ":020000040001F9\n"
      ":04000000E010E0210B\n"
      ":00000001FF\n";

  FlatMachine machine(0x20000);
  LoadResult loaded;
  std::string error;
  CHECK(LoadIntelHex(hex, machine, loaded, error));
  CHECK(loaded.format == FileFormat::kIntelHex);
  CHECK(loaded.bytes == 4);
  CHECK(loaded.MinAddress() == 0x10000);
  CHECK(loaded.MaxAddress() == 0x10004);
  CHECK(loaded.Contains(0x10000));
  CHECK(!loaded.Contains(0x10004));
  CHECK(loaded.NextMapped(0) == 0x10000);
  CHECK(machine.Read8(0x10000) == 0xE0);
  CHECK(machine.Read8(0x10001) == 0x10);
  CHECK(machine.Read8(0x10002) == 0xE0);
  CHECK(machine.Read8(0x10003) == 0x21);

  // Испорченная контрольная сумма.
  const char* bad_hex =
      ":020000040001F9\n"
      ":04000000E010E0210C\n"
      ":00000001FF\n";
  CHECK(!LoadIntelHex(bad_hex, machine, loaded, error));

  // Сырой бинарник: ровно три инструкции из примера.
  const std::vector<Byte> raw = {0xE0, 0x10, 0xE0, 0x21, 0x08, 0x1E};
  CHECK(LoadBinary(raw, 0x100, machine, loaded, error));
  CHECK(loaded.format == FileFormat::kRawBinary);
  CHECK(loaded.bytes == raw.size());
  CHECK(loaded.MinAddress() == 0x100);
  CHECK(loaded.MaxAddress() == 0x106);

  Fetcher fetcher(machine);
  Instruction instruction = fetcher.Fetch(CodeAddress{0, 0x100});
  CHECK(instruction.ToString() == "mov R0, #0x0001");
  instruction = fetcher.Fetch(CodeAddress{0, 0x102});
  CHECK(instruction.ToString() == "mov R1, #0x0002");
  instruction = fetcher.Fetch(CodeAddress{0, 0x104});
  CHECK(instruction.ToString() == "add R1, [R2+]");

  // OMF166 от Keil: файл начинается с record type 0x70 и содержит "C166".
  const std::vector<Byte> omf = {0x70, 0xD7, 0x00, 0xFF, '/', 'C', '1', '6', '6'};
  CHECK(DetectFormat(omf) == FileFormat::kOmf166);
}

}  // namespace

int main() {
  TestGoldenVectors();
  TestConditions();
  TestErrors();
  TestLoader();

  std::printf("\n%d checks, %d failures\n", g_checks, g_failures);
  return g_failures == 0 ? 0 : 1;
}
