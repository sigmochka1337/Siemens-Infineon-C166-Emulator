# Fetcher: выборка и декодирование C166 classic

Документ описывает блок Fetcher: контракт, поддерживаемые инструкции, раскладку
полей и границы ответственности. Источники: `docs/c166-ism-v2.0-2001-03.pdf`
(C166 Instruction Set Manual V2.0), `docs/tz.typ` (ТЗ проекта) и сверка с
реальным выводом Keil A166 V5.38.2.0.

Разделы ISM: Encoding — стр. 22–30, Addressing Modes (short/long/indirect,
`bitoff`) — стр. 132–140, Detailed Description — стр. 31–131.

## Границы

`Fetcher` — чистая выборка + декодирование:

```
CSP:IP ──Fetch──> Instruction{Mnemonic, condition, operands, size}
```

- читает 2 или 4 байта по физическому адресу `(CSP << 16) | IP`;
- не меняет PC, регистры и память;
- не резолвит DPP, не читает операнды из памяти;
- не исполняет инструкцию — это делает `IExecutor`.

Байты читаются через `IMachine::Read8`. Если инструкция пересекает границу
64-КБ сегмента или память недоступна, выбрасывается `UnreadableInstruction`.

## Файлы

| Файл | Ответственность |
|---|---|
| `src/interfaces/fetcher.h` | `IFetcher::Fetch(CodeAddress) -> Instruction` |
| `src/interfaces/memory.h` | `IMachine::Read8/Read16` — источник байт |
| `src/fetcher.h/.cpp` | таблица опкодов, форматы, сборка `Instruction` |
| `src/operands.h/.cpp` | типизированные операнды |
| `src/types.h/.cpp` | `Mnemonic`, `ConditionCode`, `Instruction`, `ToString` |
| `src/exceptions.h` | `UnreadableInstruction` и ошибки адресации |
| `src/flat_machine.h/.cpp` | временная `IMachine` для тестов и CLI |
| `src/loader.h/.cpp` | Intel HEX / raw binary / определение OMF166 |
| `src/main.cpp` | демо-CLI последовательного дизассемблирования |
| `tests/fetcher_test.cpp` | golden-векторы и негативные проверки |

## Поддерживаемые инструкции

### ALU

`ADD`, `ADDB`, `ADDC`, `ADDCB`, `SUB`, `SUBB`, `SUBC`, `SUBCB`,
`CMP`, `CMPB`, `NEG`, `NEGB`, `MUL`, `MULU`, `DIV`, `DIVU`, `DIVL`, `DIVLU`.

Формы:

- `Rw, Rw` / `Rb, Rb`;
- `Rw, #data3` / `Rb, #data3` (0..7);
- `Rw, #data16` / `Rb, #data16`;
- `Rw, [Rw]`, `Rw, [Rw+]`;
- `Rw, mem` и `mem, Rw` (кроме `CMP`/`CMPB`);
- `mem, #data16` для SFR-адресов;
- `NEG Rw`, `NEGB Rb`;
- `MUL/MULU Rw, Rw`; `DIV* Rw`.

### BitProcessor

`AND`, `ANDB`, `OR`, `ORB`, `XOR`, `XORB`, `CPL`, `CPLB`,
`SHL`, `SHR`, `ASHR`, `ROL`, `ROR`.

`AND/OR/XOR` используют те же формы, что ALU. Сдвиги — `Rw, Rw` и
`Rw, #data4`. `CPL`/`CPLB` — один операнд.

### Stack + BranchUnit

`PUSH`, `POP` (`Rw` и SFR: `PSW`, `SP`, `CP`, `DPP0..3`, `MDL/MDH/MDC`,
`STKOV/STKUN`), `CALLR` (синоним `CALL`), `CALLA`, `CALLI`, `CALLS`,
`PCALL`, `RET`, `RETS`, `RETP`, `RETI`, `JMPA`, `JMPI`, `JMPR`, `JMPS`,
`JB`, `JNB`, `JBC`, `JNBS`, `TRAP`, `SRVWDT`, `BSET`, `BCLR`.

Условия переходов хранятся отдельно в `Instruction::condition`:

`cc_UC`, `cc_NET`, `cc_Z`, `cc_NZ`, `cc_V`, `cc_NV`, `cc_N`, `cc_NN`,
`cc_C`, `cc_NC`, `cc_SGT`, `cc_SLE`, `cc_SLT`, `cc_SGE`, `cc_UGT`,
`cc_ULE`.

Ассемблерные алиасы `cc_EQ`, `cc_NE`, `cc_UGE`, `cc_ULT` декодируются в
канонические `cc_Z`, `cc_NZ`, `cc_NC`, `cc_C`.

### MoveUnit

`MOV`, `MOVB`, `MOVBS`, `MOVBZ`.

`MOV`:

- `Rw, #data4` (0..15), `Rw, #data16`;
- `Rw, Rw`;
- `Rw, [Rw]`, `Rw, [Rw+]`, `Rw, [Rw+#d16]`;
- `[Rw], Rw`, `[Rw+#d16], Rw`;
- `[-Rw], Rw`;
- `[Rw], [Rw]`, `[Rw+], [Rw]`;
- `Rw, mem`, `mem, Rw`, `[Rw], mem`, `mem, [Rw]`;
- `mem, #data16` (SFR).

`MOVB` — те же формы для `Rb` (`RL0..RH7`), плюс `MOVB Rb, #data4/#data16`.

`MOVB` также: `[-Rw], Rb`, `[Rw], [Rw]`, `[Rw+], [Rw]`,
`Rb, mem`, `mem, Rb`.

`MOVBS`/`MOVBZ` — `Rw, Rb`, `Rw, mem` и `mem, Rb`.

### Дополнительно

`EXTR #1..#4`, `ATOMIC #1..#4` (префиксы диапазонов), `NOP`.

## Раскладка опкодов

### ALU / BitProcessor

База плюс смещение формы:

| Операция | База |
|---|---|
| `ADD` / `ADDB` | `0x00` / `0x01` |
| `ADDC` / `ADDCB` | `0x10` / `0x11` |
| `SUB` / `SUBB` | `0x20` / `0x21` |
| `SUBC` / `SUBCB` | `0x30` / `0x31` |
| `CMP` / `CMPB` | `0x40` / `0x41` |
| `XOR` / `XORB` | `0x50` / `0x51` |
| `AND` / `ANDB` | `0x60` / `0x61` |
| `OR` / `ORB` | `0x70` / `0x71` |

Смещения:

| Смещение | Форма |
|---|---|
| `+0` | `reg, reg` |
| `+2` | `reg, mem` |
| `+4` | `mem, reg` (кроме `CMP`/`CMPB`) |
| `+6` | `reg, #data16` или `mem, #data16` (SFR) |
| `+8` | `reg, #data3` / `reg, [R0..R3]` / `reg, [R0..R3+]` |

`NEG` = `81`, `NEGB` = `A1`, `CPL` = `91`, `CPLB` = `B1`.
`MUL` = `0B`, `MULU` = `1B`.
`DIV` = `4B`, `DIVU` = `5B`, `DIVL` = `6B`, `DIVLU` = `7B`.
`SHL` = `5C` (по `#data4`) и `4C` (по `Rw`); `SHR` = `7C`/`6C`;
`ASHR` = `BC`/`AC`; `ROL` = `1C`/`0C`; `ROR` = `3C`/`2C`.

### MOV

| Опкод | Форма |
|---|---|
| `E0` | `MOV Rw, #data4` |
| `E6` | `MOV Rw, #data16` / `MOV mem, #data16` |
| `F0` | `MOV Rw, Rw` |
| `A8` | `MOV Rw, [Rw]` |
| `98` | `MOV Rw, [Rw+]` |
| `B8` | `MOV [Rw], Rw` |
| `D4` | `MOV Rw, [Rw+#d16]` |
| `C4` | `MOV [Rw+#d16], Rw` |
| `F2` | `MOV Rw, mem` |
| `F6` | `MOV mem, Rw` |
| `E1` | `MOVB Rb, #data4` |
| `E7` | `MOVB Rb, #data16` / `MOVB mem, #data16` |
| `F1` | `MOVB Rb, Rb` |
| `A9` / `99` | `MOVB Rb, [Rw]` / `MOVB Rb, [Rw+]` |
| `B9` | `MOVB [Rw], Rb` |
| `F4` | `MOVB Rb, [Rw+#d16]` |
| `E4` | `MOVB [Rw+#d16], Rb` |
| `F3` | `MOVB Rb, mem` |
| `F7` | `MOVB mem, Rb` |
| `84` / `94` | `MOV [Rw], mem` / `MOV mem, [Rw]` |
| `88` / `89` | `MOV [-Rw], Rw` / `MOVB [-Rw], Rb` |
| `C8` / `D8` | `MOV [Rw], [Rw]` / `MOV [Rw+], [Rw]` |
| `C9` / `D9` | `MOVB [Rw], [Rw]` / `MOVB [Rw+], [Rw]` |
| `D0` / `C0` | `MOVBS/MOVBZ Rw, Rb` |
| `D2` / `C2` | `MOVBS/MOVBZ Rw, mem` |
| `D5` / `C5` | `MOVBS/MOVBZ mem, Rb` |

### Ветвление и стек

| Опкод | Форма |
|---|---|
| `BB` | `CALLR rel8` |
| `DA` | `CALLS seg, caddr` |
| `CA` | `CALLA cc, caddr` |
| `AB` | `CALLI cc, [Rw]` |
| `E2` | `PCALL Rw, caddr` |
| `CB` / `DB` / `EB` / `FB` | `RET` / `RETS` / `RETP Rw` / `RETI` |
| `EA` | `JMPA cc, caddr` |
| `9C` | `JMPI cc, [Rw]` |
| `?D` | `JMPR cc, rel8` |
| `FA` | `JMPS seg, caddr` |
| `8A/9A/AA/BA` | `JB/JNB/JBC/JNBS bitaddr, rel8` |
| `EC` / `FC` | `PUSH Rw` / `POP Rw` |
| `9B` | `TRAP #trap` |
| `A7 58 A7 A7` | `SRVWDT` |
| `?F` / `?E` | `BSET/BCLR bitaddr` |
| `D1` | `EXTR/ATOMIC #1..#4` |

## Адреса и операнды

- `rel8` — знаковое смещение **в словах**; хранится в `RelativeOperand`.
- `caddr` — 16-битный адрес внутри текущего сегмента (`CodeAddressOperand`).
- `seg,caddr` — `SegmentedAddressOperand`.
- `bitaddr` — `BitOperand` (word address + bit). В короткой форме `byte1` —
  это `bitoff`: для `00..7F` база `0xFD00`, для `80..EF` база `0xFF00`,
  для `F0..FF` адрес зависит от `CP` (декодер его не знает).
- `mem` — `DirectMemoryOperand` (16-битный логический адрес).
- `[Rw]`, `[Rw+]`, `[Rw+#d16]` — `IndirectMemoryOperand`.
- `Rw` — `RegisterOperand` с `RegisterId::kR0..kR15`.
- `Rb` — `RegisterOperand` с `kRL0..kRH7` (индекс: `RL0=0`, `RH0=1`, ...).

## Ошибки

`DecodeError`/исключения:

- `UnreadableInstruction` — неизвестный опкод, недоступная память, пересечение
  границы сегмента или нарушение fixed-полей (`0xF` в старшем ниббле,
  `NOP/RET` с ненулевым вторым байтом, `DIV` с разными нибблами и т. п.).
- `PhysicalAddressOutOfRange` — адрес вне памяти `IMachine`.

## Тесты

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

`tests/fetcher_test.cpp` содержит 367 проверок: golden-векторы для каждой
группы инструкций, все 16 condition-кодов, fixed-field-ошибки, границу
64-КБ сегмента, Intel HEX/raw-загрузку и три инструкции из примера
`examples/three_instructions.bin`.

## Ограничения

- `CALL` и `CALLR` кодируются одинаково (`BB`); декодер печатает `callr`.
- `EXTS/EXTP/EXTSR/EXTPR` пока не декодируются (в ТЗ v1 не входят).
- Нет 6/8-байтовых инструкций: все формы ТЗ занимают 2 или 4 байта.
- `Instruction` не хранит исходные байты; для трассировки нужно добавить
  `raw`-поле отдельным изменением общего контракта.
- `FlatMachine` — заглушка Memory: DPP/MMIO/SFR-поведение не моделируется.
