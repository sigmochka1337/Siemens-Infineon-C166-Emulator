# Первый сквозной запуск

Сборка и запуск из корня проекта:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
./build/c166_demo
```

Ожидаемый вывод:

```text
instructions=3 R0=3 R1=2 IP=10
```

Пример в `tools/demo.cpp` связывает Machine, Fetcher, Decoder, Executor,
CpuCore и Debugger. Он загружает в адрес 0 кодировки из ISM:

```text
0:0000  E6 F0 01 00  MOV R0,#1
0:0004  E6 F1 02 00  MOV R1,#2
0:0008  00 01        ADD R0,R1
0:000A  0D FF        JMPR cc_UC,-1
```

`Run({100, CodeAddress{0,10}})` останавливается перед последней инструкцией.
Без адреса остановки этот JMPR зациклится, а лимит завершит Run с причиной
`kStepLimit`. Breakpoint также останавливает до инструкции; `Step()` позволяет
выполнить её, игнорируя breakpoint.

Начальные регистры можно задать через `core.WriteRegister(...)`, точку входа —
через `core.SetEntryPoint(...)`. Для просмотра доступны `Snapshot`, `PeekMemory`
и `InspectInstruction`; никакая из этих операций не исполняет команду.

Это демонстрация API, а не интерактивный CLI или полная поддержка C166.
