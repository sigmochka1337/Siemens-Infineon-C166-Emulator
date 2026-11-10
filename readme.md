# Сборка

```bash
cmake -E remove_directory '/home/sanya/Desktop/hsse/1 sem/pt/166/build'
cmake -S '/home/sanya/Desktop/hsse/1 sem/pt/166' -B '/home/sanya/Desktop/hsse/1 sem/pt/166/build' -G Ninja
cmake --build '/home/sanya/Desktop/hsse/1 sem/pt/166/build'
```

# Запуск

```bash
./build/c166
```

# Программа

В демо демонстрируется следующий код

```ASM
sum:
  ADD   R0, R1
  RET

main:
  MOV   R0, #1
  MOV   R1, #2
  CALLA cc_UC, sum
  MOV   R0, #0

end:
  JMPR cc_UC, end
```

Hexdump:

```HEX
000000      FA  00      ; jmps 0x00:0x0204
000002      04  02      ;
000004      00  00
  ...       00  00
0001FE      00  00
000200      00  01      ; add R0, R1
000202      CB  00      ; ret
000204      E0  10      ; mov R0, #1
000206      E0  21      ; mov R1, #2
000208      CA  00      ; calla cc_UC, 0x0200
00020A      00  02      ;
00020C      E0  00      ; mov R0, #0
000212      0D  FF      ; jmpr cc_UC, -1
```
