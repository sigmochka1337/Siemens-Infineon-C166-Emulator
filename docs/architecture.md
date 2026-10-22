# Архитектура эмулятора C166

Сценарий доклада и расширенные визуализации находятся в
[`docs/architecture_presentation.md`](architecture_presentation.md).

Каркас фиксирует границы, по которым четыре участника могут работать
параллельно. Заголовки в `src2/interfaces/` являются API; реализация конкретного
модуля может меняться без изменения соседних компонентов.

## Владение и зависимости

```mermaid
flowchart TD
  Frontend["CLI / Debugger"] --> Core["ICPUCore"]
  Core --> Fetch["IFetcher"]
  Core --> Decode["IDecoder"]
  Core --> Executor["IExecutor"]
  Fetch --> Memory["IMemoryBus"]
  Decode --> Memory
  Decode --> Registers["IRegisterBus"]
  Executor --> Memory
  Executor --> Registers
```

* **Fetch + Decode** читает окно байт и строит `Instruction`; регистры, PSW и
  память не изменяются.
* **Executor** реализует семантику инструкций, переходы, стек и флаги через
  `IRegisterBus` и `IMemoryBus`.
* **Memory** — единственный способ доступа к физической памяти. Здесь
  проверяются границы, endian, DPP, права регионов и MMIO.
* **CLI + Debugger** хранит брейкпоинты и управляет `Tick`, `run`, `step`,
  snapshot и trace, но не исполняет opcode.
* **Trace** получает записи после шага и может писать весь запуск или N шагов.

## Поток одного шага

```mermaid
sequenceDiagram
  participant D as Debugger
  participant K as ICPUCore
  participant F as IFetcher
  participant X as IDecoder
  participant E as IExecutor
  participant M as IMemoryBus
  D->>K: Tick()
  K->>F: Fetch(CSP:IP, M)
  F->>M: Read8(code address)
  M-->>F: bytes / FetchError
  F-->>K: RawInstruction
  K->>X: Decode(raw, M, registers, constants)
  X-->>K: Instruction
  K->>E: Execute(instruction, M, registers, constants)
  E->>M: Read/Write data or stack
  E-->>K: ExecutionResult
  K-->>D: result / snapshot
  D->>D: breakpoint / trace
```

## Отладочные операции

| Команда | Контракт | Семантика |
| --- | --- | --- |
| breakpoint set/remove | `addBreakpoint` / `removeBreakpoint` | остановка до инструкции по адресу |
| step N | `step(N)` | не более N инструкций |
| run | `run()` | продолжение до брейкпоинта, halt или ошибки |
| step in | `stepInto()` | одна инструкция, включая вход в CALL |
| step out | `stepOut()` | выполнение до возврата из текущего вызова |
| trace | `ITraceSink` | состояние после каждого шага |
| inspect/write | `snapshot`, `readMemory`, `writeMemory`, `writeRegister` | просмотр и изменение |

`stepOut()` требует от Executor определить глубину вызовов или адрес возврата;
это намеренно оставлено в реализации Executor, а не зашито в Debugger.

## Проверка стиля

Для C++ используется `.clang-format` на базе Google Style с шириной строки
100 символов. Локальная проверка выполняется командой:

```bash
./scripts/check-format.sh
```

Та же проверка запускается в GitHub Actions workflow `style` для каждого push и
pull request. Исправить форматирование всех исходников можно командой:

```bash
clang-format -i $(find src tests tools -type f \( -name '*.cpp' -o -name '*.h' \))
```
