# チュートリアル 13 - デバッガ

例外ハンドラにインタラクティブなデバッガを実装しよう。今や、printf()が
あるので難しくはないでしょう（より強力なマルチプラットフォームライブラリに
ついては[mini debugger](https://gitlab.com/bztsrc/minidbg)を参照して
ください）。

```sh
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Synchronous: Breakpoint instruction
> x
0007FFF0: 13 60 09 00  00 00 00 00  24 10 20 3F  00 00 00 00  .`......$. ?....
> i x30 x30+64
00080804: D2800000      movz      x0, #0x0
00080808: 94003D1C      bl        0x8FC78
0008080C: 94003ECF      bl        0x90348
00080810: D69F03E0      eret
00080814: D503201F        27 x nop
>
```

## dbg.h, dbg.c

非常にミニマルでシンプルなデバッガ(C言語で約300行)です。

`breakpoint`は、新しく定義されたキーワードです。デバッガを起動したい
コードのどこにでも使用できます。

`dbg_decodeexc()`は、チュートリアル11のexc_handler に似ていますが、
例外の原因をデコードして表示します。

`dbg_getline()`。これまで欠けていたもう一つの低レベルライブラリ関数です。
ユーザがコマンドラインを編集し、<kbd>Enter</kbd>キーを押した時に
コマンドラインを文字列として返すような方法が必要でした。いつものように
最小限の実装です。

`dbg_getoffs()`。この関数はコマンドラインの引数を解析します。
"register+/-offset"フォーマットの16進数、10進数を受け付けます。

`dbg_main()`は、デバッガのメインループです。

## disasm.h

小さい(~64k)が、すべてのARMv8.2命令をサポートしており、sprintf()にしか依存
していない（私たちは既に持っています）ので、[Universal Disassembler](https://gitlab.com/bztsrc/udisasm)を
このチュートリアルで使用することにしました。もし、デバッガにディスアセンブラを
含めたくない場合は、dbg.cの先頭でDISASSEMBLERを0に定義するだけです。

## start

`_vector`テーブルも少し変更しました。`dbg_saveregs`でレジスタをメモリに
保存し、例外の原因をプリントアウトし、ミニデバッガのメインループを
呼び出します。

## main

新しい`breakpoint`キーワードをCでテストします。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c dbg.c -o dbg.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c sprintf.c -o sprintf.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o dbg.o main.o mbox.o sprintf.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Synchronous: Breakpoint instruction
> h
Mini debugger commands:
  ?/h           this help
  r             dump registers
  x [os [oe]]   examine memory from offset start (os) to offset end (oe)
  i [os [oe]]   disassemble instruction from offset start to offset end
  c             continue execution
> x
0007FFF0: 0C F0 08 00  00 00 00 00  D4 00 08 00  00 00 00 00  ................
> i x30 x30+64
0008F00C: D4200000      brk       #0x0
0008F010: 94000184      bl        0x8F620
0008F014: 12001C00      and       w0, w0, #0xFF000000FF
0008F018: 94000176      bl        0x8F5F0
0008F01C: 17FFFFFD      b         0x8F010
0008F020: 12000C00      and       w0, w0, #0xF0000000F
0008F024: 90000024      adrp      x4, 0x8F028
0008F028: 910C8082      add       x2, x4, #0x320
0008F02C: 2A020002      orr       w2, w0, w2
0008F030: D2971301      movz      x1, #0xFFFFB898
0008F034: F2A7E001      movk      x1, #0x3F00, lsl #16
0008F038: D503201F        1 x nop
0008F03C: B9400020      ldr       w0, [x1]
0008F040: 37FFFFC0      tbnz      x0, #0x1F, 0x8F038
0008F044: D2971400      movz      x0, #0xFFFFB8A0
0008F048: F2A7E000      movk      x0, #0x3F00, lsl #16
> i 0 0+16
00000000: 580000C0      ldr       x0, 0x18
> i 0 64
00000000: 580000C0      ldr       x0, 0x18
00000004: AA1F03E1      orr       x1, xzr, xzr
00000008: AA1F03E2      orr       x2, xzr, xzr
0000000C: AA1F03E3      orr       x3, xzr, xzr
00000010: 58000084      ldr       x4, 0x20
00000014: D61F0080      br        x4
00000018: 00000100      ?
0000001C: 00000000      ?
00000020: 00080000      ?
00000024: 00000000      ?
00000028: 00000000      ?
0000002C: 00000000      ?
00000030: 00000000      ?
00000034: 00000000      ?
00000038: 00000000      ?
0000003C: 00000000      ?
> x 0x80000 0x80040
00080000: A1 00 38 D5  21 04 40 92  61 00 00 B4  5F 20 03 D5  ..8.!.@.a..._ ..
00080010: FF FF FF 17  61 4C 00 58  40 42 38 D5  00 04 7E 92  ....aL.X@B8...~.
00080020: 1F 30 00 F1  01 01 00 54  22 B6 80 D2  02 11 1E D5  .0.....T".......
00080030: 22 79 80 D2  02 40 1E D5  62 00 00 10  22 40 1E D5  "y...@..b..."@..
> i 0x80000 0x80040
00080000: D53800A1      mrs       x1, MPIDR_EL1
00080004: 92400421      and       x1, x1, #0x3
00080008: B4000061      cbz       x1, 0x80014
0008000C: D503205F      wfe
00080010: 17FFFFFF      b         0x8000C
00080014: 58004C61      ldr       x1, 0x809A0
00080018: D5384240      mrs       x0, CurrentEL
0008001C: 927E0400      and       x0, x0, #0xC
00080020: F100301F      subs      xsp, x0, #0xC
00080024: 54000101      b.ne      0x80044
00080028: D280B622      movz      x2, #0x5B1
0008002C: D51E1102      msr       S3_6_1_1_0, x2
00080030: D2807922      movz      x2, #0x3C9
00080034: D51E4002      msr       S3_6_4_0_0, x2
00080038: 10000062      adr       x2, 0x80044
0008003C: D51E4022      msr       S3_6_4_0_1, x2
> r
 x0: 000000003F201024   x1: 0000000000000301   x2: 0000000000000060
 x3: 000000000000000B   x4: 0000000000024000   x5: 0000000000000000
 x6: 0000000000000000   x7: 0000000000000000   x8: 0000000000000000
 x9: 0000000000000000  x10: 0000000000000000  x11: 0000000000000000
x12: 0000000000000000  x13: 0000000000000000  x14: 0000000000000000
x15: 0000000000000000  x16: 0000000000000000  x17: 0000000000000000
x18: 0000000000000000  x19: 0000000000000000  x20: 0000000000000000
x21: 0000000000000000  x22: 0000000000000000  x23: 0000000000000000
x24: 0000000000000000  x25: 0000000000000000  x26: 0000000000000000
x27: 0000000000000000  x28: 0000000000000000  x29: 000000000007FFF0
x30: 000000000008F00C  elr_el1: 8F00C  spsr_el1: 600003C4
  esr_el1: F2000000  far_el1: 0
sctlr_el1: 30D00800  tcr_el1: 0
> c
qemu-system-aarch64: terminating on signal 2 from pid 2324 (<unknown process>)

$ aarch64-none-elf-objdump -d kernel8.elf

0000000000080000 <_start>:
   80000:   d53800a1    mrs x1, mpidr_el1
   80004:   92400421    and x1, x1, #0x3
   80008:   b4000061    cbz x1, 80014 <_start+0x14>
   8000c:   d503205f    wfe
   80010:   17ffffff    b   8000c <_start+0xc>
   80014:   58004c61    ldr x1, 809a0 <_vectors+0x1a0>
   80018:   d5384240    mrs x0, currentel
   8001c:   927e0400    and x0, x0, #0xc
   80020:   f100301f    cmp x0, #0xc
   80024:   54000101    b.ne    80044 <_start+0x44>  // b.any
   80028:   d280b622    mov x2, #0x5b1                  // #1457
   8002c:   d51e1102    msr scr_el3, x2
   80030:   d2807922    mov x2, #0x3c9                  // #969
   80034:   d51e4002    msr spsr_el3, x2
   80038:   10000062    adr x2, 80044 <_start+0x44>
   8003c:   d51e4022    msr elr_el3, x2
   80040:   d69f03e0    eret

000000000008f000 <main>:
   8f000:   a9bf7bfd    stp x29, x30, [sp, #-16]!
   8f004:   910003fd    mov x29, sp
   8f008:   94000132    bl  8f4d0 <uart_init>
   8f00c:   d4200000    brk #0x0
   8f010:   94000184    bl  8f620 <uart_getc>
   8f014:   12001c00    and w0, w0, #0xff
   8f018:   94000176    bl  8f5f0 <uart_send>
   8f01c:   17fffffd    b   8f010 <main+0x10>
```

## start.Sにbugか

```
dbg_saveregs:
    str     x0, [sp, #-16]!     // push x0
    ldr     x0, =dbg_regs+8
    str     x1, [x0], #8        // dbg_regs[1]=x1
    //ldr     x1, [sp, #16]       // pop x1 (=x30)
    ldr     x1, [sp]            // pop x1 (=x0)
    str     x1, [x0, #-16]!     // dbg_regs[0]=x1 (x0)
```

[sp, #-16]!、すなわち、プレインデックスなのでspは-16され、その位置にx0は
pushされる。spは既に-16されていいるので、`ldr x1, [sp, #16]`ではx0ではなく、
この関数が呼び出される前に割り込みハンドラが設定したx30がpopされる。

```
sp      -> ------
           x0
sp + 16 -> ------
           x30
sp + 32 -> ------
```

確認のためにdbg_saveregsを呼び出す前にx0を設定してみた。

```
_vectors:
    // synchronous
    .align  7
    str     x30, [sp, #-16]!     // push x30
    movz    x0, #0xbeef
    movk    x0, #0xdead, LSL #16
    bl      dbg_saveregs
```

実行するとオリジナル版ではx0=x30となり、修正番ではx0はハンドラで指定した値となる。
ここでx0=x30とする意図はないと思われるので、多分バグであろう。

### オリジナル版

```
> r
 x0: 000000000008F00C   x1: 0000000000000301   x2: 0000000000000060
x30: 000000000008F00C  elr_el1: 8F00C  spsr_el1: 600003C4
```

### 修正版

```
> r
 x0: 00000000DEADBEEF   x1: 0000000000000301   x2: 0000000000000060
x30: 000000000008F00C  elr_el1: 8F00C  spsr_el1: 600003C4
```
