# Минимальный пример для Fetcher

Исходник `three_instructions.a66`:

```asm
MOV R0, #1
MOV R1, #2
ADD R1, [R2+]
```

A166 V5.38.2.0 выдаёт ровно 6 байт:

```text
E0 10   MOV R0, #1
E0 21   MOV R1, #2
08 1E   ADD R1, [R2+]
```

`three_instructions.bin` — это сырые байты `E0 10 E0 21 08 1E`.
`three_instructions.LST` — листинг A166 с адресами и объектным кодом.

Запуск:

```powershell
c166 -f examples/three_instructions.bin --base 0
```

Ожидаемый вывод:

```text
0x000000  mov R0, #0x0001
0x000002  mov R1, #0x0002
0x000004  add R1, [R2+]
```

Важно: в C166 нет трёхоперандной формы `ADD R1, R1, [R2+]`.
Реальная форма — `ADD R1, [R2+]`, где `R1` является и приёмником,
и первым источником. Именно её кодирует A166 в `08 1E`.

Пересобрать пример:

```powershell
cd examples
C:\Keil_v5\c166\bin\A166.EXE three_instructions.a66
```
