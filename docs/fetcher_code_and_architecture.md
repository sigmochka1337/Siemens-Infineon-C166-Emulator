# Fetcher: как устроен код и архитектура

> **Состояние ветки `fetcher`.** После коммита `7122733` из ветки удалены
> `src/flat_machine.*`, `src/loader.*` и CLI (`src/main.cpp` пустой). Ниже
> `loader`, `FlatMachine` и CLI описаны как демонстрационная инфраструктура,
> на которой проверялся Fetcher; ядро `src/fetcher.*` от них не зависит.
> Чтобы собирать `tests/fetcher_test.cpp`, эти файлы нужно вернуть.

Документ для защиты блока Fetcher перед ментором. Здесь объясняется:

- место Fetcher в эмуляторе C166;
- контракты с соседними компонентами;
- как устроен `fetcher.cpp` и зачем нужны таблица правил, `Format` и `Operand`;
- как декодируются конкретные инструкции;
- как проверялись кодировки;
- какие решения приняты и какие ограничения остались.

---

## 1. Где находится Fetcher в эмуляторе

Эмулятор C166 выполняет цикл:

```text
CSP:IP
  │
  ▼
Fetch ──► Decode ──► Instruction ──► Execute ──► WriteBack
  │                                  │
  └── читает байты из IMachine        └── меняет регистры, PSW, память
```

Fetcher отвечает только за первые два шага:

1. **Fetch** — прочитать 2 или 4 байта по физическому адресу `(CSP << 16) | IP`.
2. **Decode** — определить мнемонику, длину и операнды, вернуть `Instruction`.

Fetcher **не**:

- меняет `PC`, регистры, PSW или память;
- исполняет инструкцию;
- резолвит `DPP`, `CP` и физические адреса данных;
- читает операнды из памяти.

Это важно: Fetch/Decode — чистые функции. Их можно тестировать на массиве байт без процессора, использовать в дизассемблере и в отладчике.

---

## 2. Контракты

### 2.1. `IFetcher`

`src/interfaces/fetcher.h`:

```cpp
class IFetcher {
 public:
  virtual ~IFetcher() = default;
  virtual Instruction Fetch(CodeAddress pc) = 0;
};
```

`CodeAddress` — это `CSP:IP`:

```cpp
struct CodeAddress {
  Byte segment = 0;  // CSP
  Word offset = 0;   // IP

  PhysicalAddress ToPhysicalAddress() const {
    return (segment << 16) | offset;
  }
};
```

### 2.2. `IMachine`

Fetcher не знает, какая именно память под ним. Он работает через интерфейс:

```cpp
class IMachine {
 public:
  virtual Byte Read8(PhysicalAddress address) const = 0;
  virtual Word Read16(PhysicalAddress address) const = 0;
  // ... запись, регистры, PC, Reset, Resolve
};
```

Для Fetcher-а достаточно `Read8`. Запись и регистры нужны Executor-у и Debugger-у.

В тестах и CLI используется `FlatMachine` — временная реализация `IMachine` с плоской памятью 16 МБ. Это **не** боевой компонент Memory: DPP, MMIO и SFR там не моделируются.

### 2.3. `Instruction`

`src/types.h`:

```cpp
struct Instruction {
  Mnemonic mnemonic = Mnemonic::kUnknown;
  std::optional<ConditionCode> condition;
  std::vector<std::unique_ptr<Operand>> operands;
  Byte size = 0;

  std::string ToString() const;
};
```

Почему так:

- `mnemonic` — что за операция (`kAdd`, `kMov`, `kJmpR`, ...);
- `condition` — отдельное поле для `cc_*`, потому что условие перехода не является обычным данным операндом;
- `operands` — типизированные операнды (регистр, immediate, память, бит, rel, caddr);
- `size` — 2 или 4 байта, чтобы цикл исполнения знал, насколько сдвинуть `IP`;
- `ToString()` — единое текстовое представление для CLI и отладчика.

---

## 3. Общая схема кода

```text
main.cpp
  │
  ├── loader.cpp ──────────► IMachine (FlatMachine)
  │     Intel HEX / raw           │
  │                               ▼
  └── Fetcher::Fetch(pc) ───► ReadByte(pc + 0..3)
        │
        ├── FindRule(opcode)
        ├── switch (rule->format)
        │     ├── MakeRegister(...)
        │     ├── MakeImmediate(...)
        │     ├── MakeDirect(...)
        │     ├── MakeIndirect(...)
        │     ├── MakeBit(...)
        │     └── MakeRelative(...)
        │
        └── Instruction { mnemonic, condition, operands, size }
              │
              └── Instruction::ToString()
```

Файлы:

| Файл | Роль |
|---|---|
| `src/fetcher.cpp` | таблица правил и декодирование |
| `src/fetcher.h` | класс `Fetcher : IFetcher` |
| `src/types.h/.cpp` | `Instruction`, `Mnemonic`, `ConditionCode`, `ToString` |
| `src/operands.h/.cpp` | иерархия операндов |
| `src/loader.cpp` | загрузка Intel HEX/raw в `IMachine` |
| `src/flat_machine.cpp` | временная `IMachine` для тестов и CLI |
| `src/main.cpp` | CLI последовательного дизассемблирования |
| `tests/fetcher_test.cpp` | golden-векторы и негативные тесты |

---

## 4. Как устроен `fetcher.cpp`

### 4.1. Таблица правил, а не гигантский `switch`

В C166 один байт опкода может означать:

- ровно одну инструкцию (`0xCB` — `RET`);
- инструкцию с condition в старшем ниббле (`0x?D` — `JMPR cc, rel`);
- инструкцию с номером бита в старшем ниббле (`0x?F` — `BSET bitaddr`);
- инструкцию, у которой форма зависит от второго байта (`0x08` — `ADD` short).

Поэтому в коде есть две сущности:

```cpp
enum class Format : std::uint8_t {
  kNone,
  kRegReg,
  kRegImm4,
  kRegImm16,
  ...
};

struct Rule {
  Byte mask = 0xFF;
  Byte value = 0;
  Mnemonic mnemonic = Mnemonic::kUnknown;
  Format format = Format::kNone;
  Byte size = 0;
  CondSource condition = CondSource::kNone;
};
```

`Rule` описывает:

- `mask/value` — как узнать опкод;
- `mnemonic` — что это за инструкция;
- `format` — как разложить байты на операнды;
- `size` — 2 или 4 байта;
- `condition` — откуда взять condition-код (из опкода или из `byte1`).

Примеры:

```cpp
// точное совпадение: opcode == 0xCB
{0xFF, 0xCB, Mnemonic::kRet, Format::kNone, 2, CondSource::kNone}

// диапазон: любой opcode с младшим нибблом D
{0x0F, 0x0D, Mnemonic::kJmpR, Format::kCondRel8, 2,
 CondSource::kOpcodeHigh}

// диапазон: любой opcode с младшим нибблом F
{0x0F, 0x0F, Mnemonic::kBSet, Format::kBitSet, 2,
 CondSource::kNone}
```

`FindRule` — линейный поиск первого подходящего правила:

```cpp
const Rule* FindRule(Byte opcode) {
  for (const Rule& rule : Rules()) {
    if ((opcode & rule.mask) == rule.value) {
      return &rule;
    }
  }
  return nullptr;
}
```

Почему линейный поиск:

- правил около сотни, декодирование одной инструкции — не bottleneck;
- таблица простая и расширяемая;
- позже можно заменить на lookup по 256 опкодам + fallback для диапазонов.

### 4.2. `Format` — это не мнемоника, а способ разбора байтов

Один `Format` может использоваться разными инструкциями:

- `kRegReg` — `ADD Rw,Rw`, `MOV Rw,Rw`, `ROL Rw,Rw`, `MUL Rw,Rw`;
- `kShort` — `ADD/ADDC/SUB/SUBC/CMP/AND/OR/XOR` в короткой форме;
- `kCondRel8` — все 16 вариантов `JMPR cc,rel`.

Это уменьшает количество кода: декодер не пишет отдельный `switch` на каждую инструкцию.

Пример `kShort`:

```text
byte1 = dst[7:4] | sub[3:0]

sub = 0..7   -> ADD dst, #data3
sub = 8..11  -> ADD dst, [R0..R3]
sub = 12..15 -> ADD dst, [R0..R3+]
```

В коде:

```cpp
if (low <= 7) {
  instruction.operands.push_back(MakeImmediate(low));
} else if (low <= 11) {
  instruction.operands.push_back(MakeIndirect(low - 8, kPlain));
} else {
  instruction.operands.push_back(MakeIndirect(low - 12, kPostIncrement));
}
```

Именно поэтому `ADD R1, [R2+]` кодируется как `08 1E`:

```text
08     — opcode ADD short
1E     — dst = 1 (R1), sub = 0xE = 14
14     — попадает в 12..15
14-12  — base = 2 (R2)
       — режим post-increment
```

### 4.3. `CondSource` — условие отдельно от операндов

У переходов condition может лежать в разных местах:

```cpp
enum class CondSource : std::uint8_t {
  kNone,
  kOpcodeHigh,  // JMPR: 0x?D, condition = opcode >> 4
  kByte1High,   // CALLA/JMPA/CALLI/JMPI: condition = byte1 >> 4
};
```

`SetCondition` заполняет `Instruction::condition`:

```cpp
case CondSource::kOpcodeHigh:
  instruction.condition = static_cast<ConditionCode>(opcode >> 4);
  break;
case CondSource::kByte1High:
  instruction.condition = static_cast<ConditionCode>(byte1 >> 4);
  break;
```

Почему condition не в `operands`:

- Executor должен проверить условие до вычисления адреса;
- condition — не значение и не адрес, а флаг;
- так проще форматировать `jmpr cc_Z, +23` и проверять в тестах.

Соответствие 16 кодов:

| Код | Condition |
|---|---|
| 0 | `cc_UC` |
| 1 | `cc_NET` |
| 2 | `cc_Z` / `cc_EQ` |
| 3 | `cc_NZ` / `cc_NE` |
| 4 | `cc_V` |
| 5 | `cc_NV` |
| 6 | `cc_N` |
| 7 | `cc_NN` |
| 8 | `cc_C` / `cc_ULT` |
| 9 | `cc_NC` / `cc_UGE` |
| A | `cc_SGT` |
| B | `cc_SLE` |
| C | `cc_SLT` |
| D | `cc_SGE` |
| E | `cc_UGT` |
| F | `cc_ULE` |

Ассемблерные алиасы `cc_EQ`, `cc_NE`, `cc_UGE`, `cc_ULT` декодируются в канонические `cc_Z`, `cc_NZ`, `cc_NC`, `cc_C`.

### 4.4. Чтение байтов и граница сегмента

`ReadByte`:

```cpp
const uint32_t instruction_offset = pc.offset + offset;
if (instruction_offset > 0xFFFF) {
  throw UnreadableInstruction(pc, raw);
}
const PhysicalAddress address = (pc.segment << 16) | instruction_offset;
raw[offset] = machine.Read8(address);
```

Что здесь важно:

- адрес собирается как `CSP:IP`, а не через DPP;
- инструкция не может пересекать границу 64-КБ сегмента: перенос `IP` — ответственность цикла исполнения;
- `PhysicalAddressOutOfRange` превращается в `UnreadableInstruction` с контекстом;
- Fetcher не двигает `IP`.

`Fetch` читает сначала 2 байта, потом ещё 2 только если правило говорит `size == 4`:

```cpp
const Byte opcode = ReadByte(machine_, pc, 0, raw);
ReadByte(machine_, pc, 1, raw);

const Rule* rule = FindRule(opcode);
if (rule == nullptr) throw UnreadableInstruction(pc, raw);

if (rule->size == 4) {
  ReadByte(machine_, pc, 2, raw);
  ReadByte(machine_, pc, 3, raw);
}
```

Для форм ТЗ этого достаточно: все они занимают 2 или 4 байта. 6/8-байтовые формы в текущий набор не входят.

### 4.5. Валидация fixed-полей

C166 любит «зашитые» поля. Примеры:

- `E6`/`F2`/`F6`: старший ниббл `byte1` должен быть `0xF`, иначе это не GPR-форма;
- `NOP`/`RET`/`RETS`: второй байт должен быть `0x00`;
- `DIV`: номер регистра в старшем и младшем нибблах должен совпадать;
- `RETP`/`PCALL`/`PUSH`/`POP`: либо `0xF` + GPR, либо известный SFR;
- `SRVWDT`: фиксированные байты `A7 58 A7 A7`;
- `RETI`: фиксированный `FB 88`;
- `CALLA`/`JMPA`: младший ниббл `byte1` должен быть 0.

Если поле не совпало — `UnreadableInstruction`. Это лучше, чем молча выдать «почти правильную» инструкцию.

### 4.6. Типизированные операнды

`src/operands.h`:

- `RegisterOperand` — `R0..R15` или `RL0..RH7`;
- `ImmediateOperand` — `#data4/#data8/#data16`;
- `DirectMemoryOperand` — 16-битный логический адрес;
- `IndirectMemoryOperand` — `[Rw]`, `[Rw+]`, `[-Rw]`, `[Rw+#d16]`;
- `BitOperand` — `bitaddr` (word address + bit);
- `RelativeOperand` — `rel8` в словах;
- `CodeAddressOperand` — `caddr`;
- `SegmentedAddressOperand` — `seg:caddr`.

Почему `unique_ptr<Operand>`:

- операнды разнородные;
- их количество и тип зависят от инструкции;
- `Instruction` должен быть перемещаемым и полиморфным;
- Executor сможет сделать `dynamic_cast` или посетить операнды.

Альтернатива — `std::variant<...>`, но текущий интерфейс уже полиморфный, и добавление нового типа операнда не ломает `Instruction`.

Каждый операнд сам умеет `ToString`, поэтому CLI и отладчик не дублируют форматирование.

---

## 5. Разбор трёх инструкций по шагам

### `MOV R0, #1` — `E0 10`

```text
E0 — правило: MOV, Format::kRegImm4, size 2
10 — high = 1, low = 0

kRegImm4:
  operand[0] = Register(low)   = R0
  operand[1] = Immediate(high) = 1
```

Результат:

```text
mov R0, #0x0001
```

### `MOV R1, #2` — `E0 21`

```text
E0 — MOV, kRegImm4
21 — high = 2, low = 1

operand[0] = R1
operand[1] = #2
```

### `ADD R1, [R2+]` — `08 1E`

```text
08 — ADD, Format::kShort
1E — dst = 1 (R1), sub = 14

14 >= 12:
  base = 14 - 12 = 2 (R2)
  mode = post-increment
```

Результат:

```text
add R1, [R2+]
```

Важно: в C166 нет трёхоперандной формы `ADD R1, R1, [R2+]`.
`R1` — одновременно приёмник и первый источник, поэтому ассемблерная форма
двухоперандная: `ADD R1, [R2+]`.

### `JMPR cc_Z, +23` — `2D 17`

```text
2D — mask 0x0F, value 0x0D -> JMPR
     condition = opcode >> 4 = 2 -> cc_Z
17 — RelativeOperand(0x17) = +23
```

### `JB 0xFFC4.10, +6` — `8A E2 06 A0`

```text
8A — JB, Format::kBitRel, size 4
E2 — bitoff; BitWordAddress(E2) = 0xFF00 + 2*(E2-80) = 0xFFC4
06 — rel8 = +6
A0 — bit = A0 >> 4 = 10
```

### `MOV P3, #0x1234` — `E6 E2 34 12`

```text
E6 — MOV, Format::kRegImm16, size 4
E2 — high != 0xF -> это не GPR, а короткий прямой адрес SFR
     HalfWordAddress(E2) = 0xFE00 + 2*E2 = 0xFFC4 (P3)
34 12 — data16 = 0x1234
```

Результат:

```text
mov 0xFFC4, #0x1234
```

---

## 6. Загрузчик и CLI

### 6.1. `loader.cpp`

Загрузчик не декодирует. Он только кладёт байты в `IMachine`.

Поддерживает:

- Intel HEX:
  - `00` — данные;
  - `01` — EOF;
  - `02` — extended segment address;
  - `03` — start segment address;
  - `04` — extended linear address;
  - `05` — start linear address;
  - проверка контрольной суммы;
  - диапазоны загруженных адресов;
- raw binary с базовым адресом;
- определение absolute OMF166:
  - если файл начинается с `0x70` и содержит `C166`, загрузчик говорит,
    что это OMF166, и предлагает сконвертировать через `OH166` в `.H86`.

`LoadResult` хранит:

- формат;
- список диапазонов `[begin, end)`;
- количество байт;
- entry point, если он был в HEX.

Диапазоны нужны CLI, чтобы не декодировать «дырки» между секциями.

### 6.2. `main.cpp`

CLI:

```text
c166 -f FILE [--base ADDR] [--start ADDR] [--count N]
```

Алгоритм:

1. загрузить файл в `FlatMachine`;
2. выбрать стартовый адрес (`--start`, entry point или минимальный адрес);
3. в цикле:
   - пропустить немаппированные адреса через `LoadResult::NextMapped`;
   - `Fetcher::Fetch(FromPhysical(address))`;
   - напечатать `address + Instruction::ToString()`;
   - сдвинуть адрес на `instruction.size`;
4. остановиться на `--count`, конце диапазона или ошибке.

CLI не знает про опкоды: он только вызывает Fetcher и печатает `Instruction`.

---

## 7. Как проверялись кодировки

Источники:

1. `docs/c166-ism-v2.0-2001-03.pdf` — C166 Instruction Set Manual V2.0:
   - Encoding, стр. 22–30;
   - Addressing Modes, стр. 132–140;
   - Detailed Description, стр. 31–131.
2. Keil A166 V5.38.2.0 — реальный ассемблер C166.

Почему A166:

- мануал даёт формат, но ошибку в одном байте легко не заметить;
- A166 — эталонный ассемблер, который реально собирает C166;
- листинг `.LST` показывает адрес, байты и исходную инструкцию.

Пример:

```asm
MOV R0, #1
MOV R1, #2
ADD R1, [R2+]
```

A166 выдаёт:

```text
00000000 E010   MOV R0, #1
00000002 E021   MOV R1, #2
00000004 081E   ADD R1, [R2+]
```

Эти байты стали golden-векторами в `tests/fetcher_test.cpp`.

---

## 8. Тесты

`tests/fetcher_test.cpp`:

- golden-векторы для каждой группы инструкций;
- все 16 condition-кодов (`JMPR`);
- fixed-field-ошибки:
  - неизвестный опкод;
  - `NOP` с ненулевым вторым байтом;
  - `NEG` с младшим нибблом не 0;
  - `DIV` с разными нибблами;
  - `RETP` с неизвестным special-регистром;
  - `CALLA` с мусором в `byte1`;
  - `SRVWDT` с неверными фиксированными байтами;
  - `RETI` с неверным вторым байтом;
- граница 64-КБ сегмента;
- `Fetch` не меняет `PC`;
- загрузка Intel HEX:
  - корректная запись;
  - неверная контрольная сумма;
  - расширенный линейный адрес;
- raw binary;
- OMF166 detection.

Сейчас:

```text
367 checks, 0 failures
```

CMake/CTest:

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

---

## 9. Принятые решения и компромиссы

### 9.1. Fetcher не работает с `Machine` как с состоянием

Fetcher получает `const IMachine&` и вызывает только `Read8`. Это гарантирует,
что декодирование не меняет состояние и его можно использовать в отладчике и
дизассемблере.

### 9.2. Condition — отдельное поле

Условие перехода не является операндом данных. Его проверяет Executor до
вычисления адреса. Отдельное поле позволяет не смешивать `cc_Z` с `caddr` и
`rel`.

### 9.3. Операнды полиморфные

`Instruction` хранит `vector<unique_ptr<Operand>>`. Это даёт:

- разные типы операндов в одной инструкции;
- `ToString` без большого `switch` в CLI;
- расширение без изменения `Instruction`.

Минус — dynamic allocation на каждую инструкцию. Для эмулятора это не
критично; позже можно перейти на `std::variant` или арену.

### 9.4. Таблица правил с `mask/value`

Позволяет одной записью описать диапазон `0x?D` для `JMPR` и `0x?F` для
`BSET`, не заводя 16 почти одинаковых записей. Точные опкоды описываются
`mask = 0xFF`.

### 9.5. Ошибка — исключение

`UnreadableInstruction` содержит `CodeAddress` и первые 4 байта контекста.
Это соответствует требованию ТЗ «остановка при неподдерживаемой инструкции»:
цикл исполнения ловит исключение и останавливает эмулятор.

---

## 10. Ограничения

- `CALL` и `CALLR` кодируются одинаково (`BB`), поэтому декодер печатает
  `callr`.
- `EXTS/EXTP/EXTSR/EXTPR` не реализованы: в ТЗ v1 они не входят.
- `bitoff` `F0..FF` зависит от `CP`; Fetcher его не резолвит.
- 6/8-байтовых инструкций в текущем наборе ТЗ нет, поэтому `raw` — 4 байта.
- `FlatMachine` — заглушка Memory: DPP/MMIO/SFR не моделируются.
- `Instruction` не хранит исходные байты; для трассировки это отдельное
  изменение общего контракта.

---

## 11. Вопросы, которые может задать ментор

**Почему декодирование не в Executor?**

Потому что Fetch/Decode — чистые и тестируемые. Executor получает уже
типизированную `Instruction` и занимается только семантикой. Это разделяет
ответственность и позволяет использовать Fetcher в дизассемблере.

**Почему таблица, а не `switch` по опкодам?**

Таблица данных проще расширяется и позволяет описывать диапазоны через
`mask/value`. `switch` на 100+ опкодов быстро стал бы нечитаемым.

**Почему линейный поиск?**

Правил около сотни, декодирование одной инструкции не является узким местом.
Если понадобится, заменим на массив из 256 указателей + fallback для
диапазонов.

**Почему `unique_ptr<Operand>`, а не `std::variant`?**

Исторически интерфейс операндов полиморфный; `unique_ptr` позволяет хранить
разные типы и легко добавлять новые. `variant` тоже возможен, но потребует
переписать `Instruction` и все фабрики.

**Как гарантируется, что Fetcher ничего не меняет?**

Он принимает `const IMachine&`, вызывает только `Read8`, не вызывает
`SetPC/Write8/WriteRegister`. В тестах есть отдельная проверка, что `PC` не
изменился.

**Что будет при неподдерживаемой инструкции?**

`UnreadableInstruction` с адресом и контекстом. Цикл исполнения должен
остановить эмулятор и показать диагностику.

**Как проверялись кодировки?**

По C166 ISM V2.0 (Encoding, стр. 22–30) и по реальному выводу Keil A166
V5.38.2.0. Байты из листингов A166 зафиксированы в тестах.

**Что осталось за пределами Fetcher?**

Executor (ALU, PSW, ветвления, стек), полноценная Memory с DPP/MMIO, Debugger,
CLI-команды Step/Run/Breakpoints и трассировка. Fetcher — первый блок, на
который они опираются.

---

## 12. Быстрый запуск для демонстрации

```powershell
# собрать CLI напрямую (ASCII-путь или без кириллицы в пути)
g++ -std=c++23 -Isrc `
    src/types.cpp src/operands.cpp src/fetcher.cpp `
    src/flat_machine.cpp src/loader.cpp src/main.cpp `
    -o c166.exe

# три инструкции
.\c166.exe -f examples\three_instructions.bin --base 0
```

Ожидаемо:

```text
0x000000  mov R0, #0x0001
0x000002  mov R1, #0x0002
0x000004  add R1, [R2+]
```

Реальный C166 HEX из Keil:

```powershell
.\c166.exe -f Hello.H86 --start 0xC00000 --count 20
```

---

## 13. Итог

Fetcher — это слой между памятью и Executor:

```text
байты C166 ──► Fetch/Decode ──► Instruction ──► Execute
```

Он:

- читает ровно 2/4 байта по `CSP:IP`;
- не имеет побочных эффектов;
- описывает инструкцию типизированными операндами;
- проверяет fixed-поля;
- отдаёт `condition` отдельно;
- покрыт golden-векторами из официального мануала и A166.

Этого достаточно, чтобы декодировать прошивку C166 и использовать Fetcher в
дизассемблере/отладчике; для исполнения нужен следующий блок — Executor.
