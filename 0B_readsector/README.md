# チュートリアル 0B - セクターの読み込み

これまでのデータ(pixmap, font)をカーネルイメージにリンクしました。
いよいよSDカードからデータを読み込みます。このチュートリアルでは
セクタの読み取り機能を持つ実際のドライバを実装します。

## sd.h, sd.c

さて、セクタを読み書きするためのメールボックスがあればいいのですが、
それはありません。そのため、EMMCと直接対話しなければなりませんが、
これは厄介で退屈な作業です。あらゆる種類のカードを扱わなければ
なりません。しかし、最終的には2つの関数を用意しました。

`sd_init()`は、SDカードを読み込むためのEMMCの初期化を行います。

`sd_readblock(lba,buffer,num)`は、SDカードのセクタlbaからnumブロック
（セクタ）をbufferに読み込みます。

## main

メモリ内のbssセグメントの後にあるブロックを読み込んで、コンソールに
ダンプします。read関数はEMMCとの通信に関する詳細な情報を表示します。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c delays.c -o delays.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c sd.c -o sd.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o delays.o main.o mbox.o sd.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
cp kernel8.img test.dd
qemu-img resize -f raw test.dd 8192
Image resized.
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -drive file=test.dd,if=sd,format=raw -serial stdio
EMMC: GPIO set up
EMMC: reset OK
sd_clk divisor 00000068, shift 00000006
EMMC: Sending command 00000000 arg 00000000
EMMC: Sending command 08020000 arg 000001AA
EMMC: Sending command 37000000 arg 00000000
EMMC: Sending command 29020000 arg 51FF8000
EMMC: CMD_SEND_OP_COND returned COMPLETE VOLTAGE FFFFFFFF80F9C000
EMMC: Sending command 02010000 arg 00000000
EMMC: Sending command 03020000 arg 00000000
EMMC: CMD_SEND_REL_ADDR returned 0000000045670000
sd_clk divisor 00000002, shift 00000000
EMMC: Sending command 07030000 arg 45670000
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 33220010 arg 00000000
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 06020000 arg 45670002
EMMC: supports
sd_readblock lba 00000000 num 00000001
EMMC: Sending command 11220010 arg 00000000
00081780: A1 00 38 D5  21 04 40 92  61 00 00 B4  5F 20 03 D5  ..8.!.@.a..._ ..
00081790: FF FF FF 17  61 01 00 58  3F 00 00 91  61 01 00 58  ....a..X?...a..X
000817A0: E2 00 00 18  82 00 00 34  3F 84 00 F8  42 04 00 51  .......4?...B..Q
000817B0: A2 FF FF 35  53 00 00 94  F5 FF FF 17  18 00 00 00  ...5S...........
000817C0: 00 00 08 00  00 00 00 00  C0 16 08 00  00 00 00 00  ................
000817D0: C0 00 00 34  00 04 00 51  1F 20 03 D5  00 04 00 51  ...4...Q. .....Q
000817E0: 1F 04 00 31  A1 FF FF 54  C0 03 5F D6  1F 20 03 D5  ...1...T.._.. ..
000817F0: E2 03 00 2A  01 E0 3B D5  20 E0 3B D5  E3 F9 9E D2  ...*..;. .;.....
00081800: 63 6A BC F2  21 FC 43 D3  A3 74 D3 F2  83 18 E4 F2  cj..!.C..t......
00081810: 21 7C C3 9B  21 FC 44 D3  21 7C 02 9B  21 FC 43 D3  !|..!.D.!|..!.C.
00081820: 21 7C C3 9B  00 10 41 8B  21 E0 3B D5  1F 00 01 EB  !|....A.!.;.....
00081830: C8 FF FF 54  C0 03 5F D6  1F 20 03 D5  1F 20 03 D5  ...T.._.. ... ..
00081840: 02 01 86 D2  02 E0 A7 F2  83 00 86 D2  03 E0 A7 F2  ................
00081850: 40 00 40 B9  61 00 40 B9  44 00 40 B9  9F 00 00 6B  @.@.a.@.D.@....k
00081860: 60 00 00 54  40 00 40 B9  61 00 40 B9  E1 03 01 2A  `..T@.@.a.@....*
00081870: 20 80 00 AA  C0 03 5F D6  1F 20 03 D5  1F 20 03 D5   ....._.. ... ..
00081880: 03 01 86 D2  03 E0 A7 F2  84 00 86 D2  04 E0 A7 F2  ................
00081890: 65 00 40 B9  82 00 40 B9  61 00 40 B9  BF 00 01 6B  e.@...@.a.@....k
000818A0: 60 00 00 54  61 00 40 B9  82 00 40 B9  E2 03 02 2A  `..Ta.@...@....*
000818B0: 41 80 01 AA  41 02 00 B4  20 40 20 8B  03 01 86 D2  A...A... @ .....
000818C0: 03 E0 A7 F2  85 00 86 D2  05 E0 A7 F2  1F 20 03 D5  ............. ..
000818D0: 64 00 40 B9  A2 00 40 B9  61 00 40 B9  9F 00 01 6B  d.@...@.a.@....k
000818E0: 60 00 00 54  61 00 40 B9  A2 00 40 B9  E2 03 02 2A  `..Ta.@...@....*
000818F0: 41 80 01 AA  3F 00 00 EB  C3 FE FF 54  C0 03 5F D6  A...?......T.._.
00081900: FD 7B BE A9  FD 03 00 91  BA 03 00 94  6D 02 00 94  .{..........m...
00081910: C0 00 00 34  1F 20 03 D5  06 04 00 94  00 1C 00 12  ...4. ..........
00081920: F8 03 00 94  FD FF FF 17  B3 0B 00 F9  13 00 00 B0  ................
00081930: 73 02 1E 91  22 00 80 52  E1 03 13 AA  41 01 00 94  s..."..R....A...
00081940: 60 00 00 35  B3 0B 40 F9  F4 FF FF 17  E0 03 13 AA  `..5..@.........
00081950: 3C 04 00 94  B3 0B 40 F9  F0 FF FF 17  00 00 00 00  <.....@.........
00081960: 00 0C 00 12  04 00 00 B0  82 00 1B 91  02 00 02 2A  ...............*
00081970: 01 13 97 D2  01 E0 A7 F2  1F 20 03 D5  20 00 40 B9  ......... .. .@.
```

```
$ aarch64-none-elf-nm kernel8.elf
0000000000081780 B __bss_end
0000000000000018 A __bss_size
00000000000816c0 B __bss_start
0000000000081780 B _end                 # <= _end = 0x81780
0000000000080000 T _start
```

```
$ vi Makefile

 run:
+   cp kernel8.img test.dd
+   qemu-img resize -f raw test.dd 8192
    qemu-system-aarch64 -M raspi3 -kernel kernel8.img -drive file=test.dd,if=sd,format=raw -serial stdio
```

### メモ

- test.ddの代わりにkernel8.imgのサイスを変更して使用することもできる。
- test.ddのサイズが2のべき乗でないと次のエラーとなる。
    ```
    $ make run
    qemu-system-aarch64 -M raspi3 -kernel kernel8.img -drive file=test.dd,if=sd,format=raw -serial stdio
    qemu-system-aarch64: Invalid SD card size: 6 KiB
    SD card size has to be a power of 2, e.g. 8 KiB.
    You can resize disk images with 'qemu-img resize <imagefile> <new-size>'
    (note that this will lose data if you make the image smaller than it currently is).
    make: *** [run] Error 1
    ```
- フォーマット指定なしでリサイズした場合、次のワーニングがでる。
    ```
    $ qemu-img resize test.dd 8192
    WARNING: Image format was not specified for 'test.dd' and probing guessed raw.
    Automatically detecting the format is dangerous for raw images, write operations on block 0 will be restricted.
    Specify the 'raw' format explicitly to remove the restrictions.
    Image resized.
    ```

## 参考資料

- [bcm2837.pas](https://github.com/ultibohub/Core/blob/master/source/rtl/ultibo/core/bcm2837.pas)
