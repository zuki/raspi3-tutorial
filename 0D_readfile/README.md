チュートリアル 0D - ファイルを読む

ルートディレクトリの読み方とパースの仕方を学びました。今回のチュートリアルでは、
ルートディレクトリからファイルを一つ取得し、そのクラスタチェーンをたどって
ファイル全体をメモリにロードします。

```sh
$ qemu-system-aarch64 -M raspi3 -drive file=test.dd,if=sd,format=raw -serial stdio
        ... output removed for clearity ...
FAT File LICENC~1BRO starts at cluster: 00000192
FAT Bytes per Sector: 00000200
FAT Sectors per Cluster: 00000004
FAT Number of FAT: 00000002
FAT Sectors per FAT: 00000014
FAT Reserved Sectors Count: 00000004
FAT First data sector: 00000054
        ... output removed for clearity ...
00085020: 43 6F 70 79  72 69 67 68  74 20 28 63  29 20 32 30  Copyright (c) 20
00085030: 30 36 2C 20  42 72 6F 61  64 63 6F 6D  20 43 6F 72  06, Broadcom Cor
00085040: 70 6F 72 61  74 69 6F 6E  2E 0A 43 6F  70 79 72 69  poration..Copyri
00085050: 67 68 74 20  28 63 29 20  32 30 31 35  2C 20 52 61  ght (c) 2015, Ra
00085060: 73 70 62 65  72 72 79 20  50 69 20 28  54 72 61 64  spberry Pi (Trad
00085070: 69 6E 67 29  20 4C 74 64  0A 41 6C 6C  20 72 69 67  ing) Ltd.All rig
        ... output removed for clearity ...
```

## fat.h, fat.c

この関数も簡単で、よく文書化されています。ファイルのディレクトリエントリを探して、
先頭のクラスタ番号を取得します。そして、クラスタチェーンをたどりながら、
各クラスターをメモリにロードします。

`fat_getpartition()`は、最初のMBRパーティションのブートレコードをロードして
検査します。

`fat_getcluster(fn)`は、指定したファイル名の先頭のクラスタを返します。

`fat_readfile(clu)`は、ファイルをメモリに読み込み、最初のバイトへのポインタを
返します。

## main

EMMCを初期化してセクタを読めるようにしてから、第1パーティションのブートレコードを
読み込みます。BPBに有効なFATパーティションが記述されていれば、ファイル`LICENCE.broadcom`の先頭のクラスタを見つけます。それが見つからない場合は`kernel8.img`を探します。どちらかのファイルが見つかれば、それをロードして、最初の512バイトをダンプします。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c delays.c -o delays.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c fat.c -o fat.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c sd.c -o sd.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o delays.o fat.o main.o mbox.o sd.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
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
EMMC: Sending command 37020000 arg 45670000q
EMMC: Sending command 33220010 arg 00000000
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 06020000 arg 45670002
EMMC: supports
sd_readblock lba 00000000 num 00000001
EMMC: Sending command 11220010 arg 00000000
sd_readblock lba 00000000 num 00000001
EMMC: Sending command 11220010 arg 00000000
sd_readblock lba 00000810 num 00000002
EMMC: Sending command 11220010 arg 00102000
EMMC: Sending command 11220010 arg 00102200
FAT File LICENC~1BRO starts at cluster: 00000005
FAT Bytes per Sector: 00000200
FAT Sectors per Cluster: 00000002
FAT Number of FAT: 00000002
FAT Sectors per FAT: 000003F8
FAT Reserved Sectors Count: 00000020
FAT First data sector: 00000810
sd_readblock lba 00000001 num 00000418
EMMC: Sending command 11220010 arg 00000200
...
EMMC: Sending command 11220010 arg 00083000
sd_readblock lba 00000816 num 00000002
EMMC: Sending command 11220010 arg 00102C00
EMMC: Sending command 11220010 arg 00102E00
sd_readblock lba 00000818 num 00000002
EMMC: Sending command 11220010 arg 00103000
EMMC: Sending command 11220010 arg 00103200
00105050: 43 6F 70 79  72 69 67 68  74 20 28 63  29 20 32 30  Copyright (c) 20
00105060: 30 36 2C 20  42 72 6F 61  64 63 6F 6D  20 43 6F 72  06, Broadcom Cor
00105070: 70 6F 72 61  74 69 6F 6E  2E 0A 43 6F  70 79 72 69  poration..Copyri
00105080: 67 68 74 20  28 63 29 20  32 30 31 35  2C 20 52 61  ght (c) 2015, Ra
00105090: 73 70 62 65  72 72 79 20  50 69 20 28  54 72 61 64  spberry Pi (Trad
001050A0: 69 6E 67 29  20 4C 74 64  0A 41 6C 6C  20 72 69 67  ing) Ltd.All rig
001050B0: 68 74 73 20  72 65 73 65  72 76 65 64  2E 0A 0A 52  hts reserved...R
...
```

## メモ

- ディレクトリエントリが19あり、1セクタではすべてを読み込めなかった（512/32=16）
  - `fat.c#L142`を修正
    ```
    ---    if(sd_readblock(root_sec,(unsigned char*)dir,s/512+1)) {
    +++    if(sd_readblock(root_sec,(unsigned char*)dir,s/512+2)) {
    ```

## 参考資料

- [FATファイルシステムのしくみと操作法](http://elm-chan.org/docs/fat.html)
