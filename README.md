# Siemens/Infineon C166 Emulator

Эмулятор процессорного ядра Siemens/Infineon C166 classic для динамического
анализа и реверс-инжиниринга прошивок. Цель — не «просто эмулятор», а
инструмент: загрузить бинарник, выполнять его по шагам, смотреть регистры,
PSW и память, останавливаться по адресу и понимать текущую инструкцию.

Техническое задание: `docs/Техническое задание.pdf`, архитектурная схема:
`docs/arch.png`.

## Состояние

| Модуль | Ответственный | Статус |
|---|---|---|
| Fetch + Decode | — | каркас готов, таблица опкодов заполняется по мануалу |
| Executor (ALU, PSW, Registers, Stack, BranchUnit) | — | не начат |
| Memory (DPP, MMIO, загрузка бинарника) | — | заголовок пустой |
| Debugger | — | не начат |

Проект в начале: есть контракт `Instruction`, рабочая выборка, каркас
декодера и тесты. Реального исполнения команд пока нет.

## Сборка

Нужен C++20-компилятор и CMake ≥ 3.20.

```bash
cmake -S . -B build
cmake --build build
```

Windows + MinGW: если в пути есть кириллица, `mingw32-make` не находит файлы —
либо переименуйте папку в ASCII-путь, либо собирайте Visual Studio / Ninja.

## Демонстрация Fetch + Decode

`c166dis` разбирает байты в инструкцию, не запуская процессор. Это тот же
путь, который использует эмулятор: память → fetch → decode → `Instruction`.

```bash
./build/c166dis 11 03 34 12
# addr 0x000100  bytes 11 03 34 12
# len  4
# asm  MOV R3, #0x1234

./build/c166dis 10 21        # ADD R1, R2
./build/c166dis 13 F0        # JMPR -16   (rel8 со знаком)
./build/c166dis -a 0x200 12 05 78 56
./build/c166dis FF FF        # error: illegal opcode (opcode 0xFF)
```

Таблица опкодов в демо **условная**: она проверяет механику разбора, но не
сверена с Instruction Set Manual, поэтому инструмент всегда печатает
предупреждение. Боевая таблица будет заполнена построчно с указанием источника.

## Тесты

```bash
ctest --test-dir build --output-on-failure
```

Сейчас 58 проверок: выборка окна, граница 64К-сегмента, ошибки памяти,
`IllegalOpcode` / `Truncated`, разбор операндов, формат строки для отладчика.

## Структура

```
src/includes/instruction.h     внутреннее представление инструкции (контракт)
src/includes/decoder.h         Fetched, DecodeTable, DecodeError, API
src/includes/memory_iface.h    предложение интерфейса памяти (read8/read16/...)
src/fetch.cpp                  выборка окна по CSP:IP
src/decode.cpp                 разбор опкодов и операндов, format(), disassembleOne()
tools/c166dis.cpp              демонстрационный CLI
tests/test_decode.cpp          тесты Fetch/Decode
docs/fetch_decode.md           решения D1–D7 и открытые вопросы
docs/isa_scope.md              перечень мнемоник v1 по компонентам ТЗ
```

## Дальше

1. Заполнить таблицу опкодов по Instruction Set Manual (v1-перечень — в
   `docs/isa_scope.md`).
2. Golden-векторы: реальные байты → ожидаемая мнемоника и длина.
3. Согласовать интерфейс памяти с владельцем Memory.
4. Минимальный цикл fetch → decode → execute на 2–3 командах.
