# Siemens/Infineon C166 Emulator

Эмулятор процессорного ядра Siemens/Infineon C166 classic для динамического
анализа и реверс-инжиниринга прошивок. Цель — загрузить бинарник, выполнять его
по шагам, смотреть регистры, PSW и память, останавливаться по адресу и понимать
текущую инструкцию.

Сейчас в репозитории реализован блок **Fetcher**: выборка и декодирование
инструкций C166. Executor, Debugger и полноценная Memory пока представлены
интерфейсами.

> **Состояние ветки `fetcher`.** После коммита `7122733` из ветки удалены
> `src/flat_machine.*`, `src/loader.*` и CLI (`src/main.cpp` пустой).
> Разделы про CLI/загрузчик ниже описывают локальную демо-конфигурацию,
> в которой проверялся Fetcher; ядро `src/fetcher.*` от них не зависит.
> Чтобы собирать `tests/fetcher_test.cpp`, эти файлы нужно вернуть.

## Состояние

| Блок | Статус |
|---|---|
| Fetcher (выборка + декодирование) | реализован по списку операций ТЗ |
| Operand-модель | `Register`, `Immediate`, `DirectMemory`, `IndirectMemory`, `Bit`, `Relative`, `CodeAddress`, `SegmentedAddress` |
| Загрузчик | Intel HEX, raw binary, определение OMF166 |
| CLI | последовательное дизассемблирование файла |
| Executor | только интерфейс `IExecutor` |
| Debugger | только интерфейс `IDebugger` |
| Memory | `IMachine` + временная `FlatMachine` для тестов |

Поддерживаются ALU, BitProcessor, Stack + BranchUnit и MoveUnit из ТЗ:
`ADD/ADDB/ADDC/ADDCB/SUB/SUBB/SUBC/SUBCB/CMP/CMPB/NEG/MUL/MULU/DIV*`,
`AND/OR/XOR/CPL/SHL/SHR/ASHR/ROL/ROR`, `PUSH/POP/CALL*/RET*/JMP*/JB/JNB/JBC/JNBS`,
`MOV/MOVB/MOVBS/MOVBZ`, `NOP/SRVWDT/TRAP/BSET/BCLR/EXTR/ATOMIC`.

Подробности и таблица опкодов: [`docs/fetcher.md`](docs/fetcher.md).

## Сборка

Нужен C++23-компилятор и CMake ≥ 3.20.

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

Windows + MinGW: если в пути есть кириллица, `mingw32-make` может не найти
сгенерированные файлы CMake. Собирайте в ASCII-пути или напрямую:

```powershell
g++ -std=c++23 -Isrc src/types.cpp src/operands.cpp src/fetcher.cpp `
    src/flat_machine.cpp src/loader.cpp src/main.cpp -o c166.exe
```

## CLI

```powershell
c166 -f examples\three_instructions.bin --base 0
```

```text
file:   examples\three_instructions.bin
format: raw
bytes:  6
0x000000  mov R0, #0x0001
0x000002  mov R1, #0x0002
0x000004  add R1, [R2+]
```

Intel HEX:

```powershell
c166 -f Hello.H86 --start 0xC00000 --count 20
```

`--base` задаёт адрес загрузки raw-бинарника, `--start` — физический адрес
первой инструкции, `--count` — ограничение числа инструкций.

## Тесты

```bash
ctest --test-dir build --output-on-failure
```

`tests/fetcher_test.cpp` содержит golden-векторы из A166, все condition-коды,
проверки fixed-полей, границу 64-КБ сегмента и загрузку Intel HEX/raw.
Сейчас проходит 367 проверок.

## Структура

```text
src/interfaces/fetcher.h    IFetcher::Fetch(CodeAddress) -> Instruction
src/interfaces/memory.h     IMachine: память, регистры, PC
src/fetcher.h/.cpp          таблица опкодов и декодирование
src/operands.h/.cpp         типизированные операнды
src/types.h/.cpp            Instruction, Mnemonic, ConditionCode, ToString
src/loader.h/.cpp           Intel HEX / raw / OMF166
src/flat_machine.h/.cpp     временная IMachine для тестов и CLI
src/main.cpp                CLI
tests/fetcher_test.cpp      тесты Fetch/Decode и загрузчика
examples/                   минимальный пример на три инструкции
docs/fetcher.md             описание блока Fetcher
```

## Документация

- `docs/c166-ism-v2.0-2001-03.pdf` — C166 Instruction Set Manual V2.0;
- `docs/c166-subs-um-v1-6-b.pdf` — C166S V1 Subsystem User Manual;
- `docs/tz.typ` / `docs/tz.pdf` — техническое задание;
- `docs/fetcher.md` — контракт Fetcher, кодировки, ограничения;
- `docs/fetcher_code_and_architecture.md` — подробный разбор кода и
  архитектуры Fetcher для защиты проекта.

## Ограничения

- `CALL` и `CALLR` имеют одинаковое кодирование (`BB`), декодер печатает `callr`.
- `EXTS/EXTP/EXTSR/EXTPR` не реализованы (в ТЗ v1 не входят).
- `bitoff` `F0..FF` зависит от `CP`; Fetcher его не резолвит.
- `FlatMachine` — заглушка Memory: DPP/MMIO/SFR не моделируются.
- 6/8-байтовых инструкций в текущем наборе ТЗ нет.
