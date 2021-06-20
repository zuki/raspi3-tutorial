# チュートリアル 0C - ディレクトリ

ストレージからセクタをロードできるようになったので、今度はそれをファイルシステムと
して解析してみましょう。今回のチュートリアルでは、FAT16またはFAT32パーティションの
ルートディレクトリエントリをリストする方法を示します。

```sh
$ qemu-system-aarch64 -M raspi3 -drive file=test.dd,if=sd,format=raw -serial stdio
        ... output removed for clearity ...
MBR disk identifier: 12345678
FAT partition starts at: 00000008
        ... output removed for clearity ...
FAT type: FAT16
FAT number of root diretory entries: 00000200
FAT root directory LBA: 00000034
        ... output removed for clearity ...
Attrib Cluster  Size     Name
...L.. 00000000 00000000 EFI System
....D. 00000003 00000000 FOLDER
.....A 00000171 0000C448 BOOTCODEBIN
.....A 0000018A 000019B3 FIXUP   DAT
.....A 0000018E 00001B10 KERNEL8 IMG
.....A 00000192 000005D6 LICENC~1BRO
.....A 00000193 002B2424 START   ELF
```

## fat.h, fat.c

このファイルは簡単で詳細に文書化されています。MBRを読み込んでパーティションを見つけ、
その第1セクタ（VBR: Volume Boot Record）をロードします。ここにはFATファイルシステムを
記述したBPS（BIOS Parameter Block）があります。

`fat_getpartition()`は、最初のMBRパーティションのブートレコードをロードしてチェックします。

`fat_listdirectory()`は、ボリューム上のルートディレクトリエントリをリストします。

## main

セクタを読むようにEMMCを初期化して、最初のパーティションのブートレコードをロードします。
BPBに有効なFATパーティションが記述されている場合は、ルートディレクトリエントリをリスト
します。

## 実行結果

```
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
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 33220010 arg 00000000
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 06020000 arg 45670002
EMMC: supports
sd_readblock lba 00000000 num 00000001
EMMC: Sending command 11220010 arg 00000000
ERROR: Wrong partition type
FAT partition not found???
```

- test.ddをhexdumpしたところ、パーティションテーブルが書かれていなかった。
        ```
        $ xxd test.dd | head -32 | tail -5
        000001b0: 0000 0000 0000 0000 0000 0000 0000 0000  ..........$.....
        000001c0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
        000001d0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
        000001e0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
        000001f0: 0000 0000 0000 0000 0000 0000 0000 55aa  ..............U.
        ```
- `--mbr=y`を付けて`mkfs.fat`を再実行（--mbrのデフォオルトはautoで、この場合、
  パーティションテーブルが書かれるのはnon-removable disksの場合のみだった）
- `--mbr=y`を付けた後
        ```
        $ xxd test.dd | head -32 | tail -5
        000001b0: 0000 0000 0000 0000 b61c 2403 0000 8000  ..........$.....
        000001c0: 0100 0cfe ffff 0000 0000 0000 0400 0000  ................
        000001d0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
        000001e0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
        000001f0: 0000 0000 0000 0000 0000 0000 0000 55aa  ..............U.
        ```

```
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
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 33220010 arg 00000000
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 06020000 arg 45670002
EMMC: supports
sd_readblock lba 00000000 num 00000001
EMMC: Sending command 11220010 arg 00000000
MBR disk identifier: 03241CB6
FAT partition starts at: 00000000
sd_readblock lba 00000000 num 00000001
EMMC: Sending command 11220010 arg 00000000
FAT type: FAT32
FAT number of root diretory entries: 00000000
FAT root directory LBA: 00000810
sd_readblock lba 00000810 num 00000001
EMMC: Sending command 11220010 arg 00102000

Attrib Cluster  Size     Name
...L.. 00000000 00000000 RASPI3
.H..D. 00000003 00000000 FSEVEN~1
.....A 00000005 0000CCE8 BOOTCODEBIN
.H...A 00000039 00001000 _BOOTC~1BIN
.....A 0000003D 00001C8E FIXUP   DAT
.H...A 00000045 00001000 _FIXU~1 DAT
.....A 00000049 00001C0C KERNEL8 IMG
.....A 00000051 002D1D20 START   ELF
.H...A 00000B99 00001000 _STAR~1 ELF
```

```
$ xxd test.dd | head -128
00000000: eb58 906d 6b66 732e 6661 7400 0202 2000  .X.mkfs.fat... .
00000010: 0200 0000 00f8 0000 2000 0800 0000 0000  ........ .......
00000020: 0000 0400 f803 0000 0000 0000 0200 0000  ................
00000030: 0100 0600 0000 0000 0000 0000 0000 0000  ................
00000040: 8000 298a 1a24 0352 4153 5049 3320 2020  ..)..$.RASPI3
00000050: 2020 4641 5433 3220 2020 0e1f be77 7cac    FAT32   ...w|.
00000060: 22c0 740b 56b4 0ebb 0700 cd10 5eeb f032  ".t.V.......^..2
00000070: e4cd 16cd 19eb fe54 6869 7320 6973 206e  .......This is n

$ xxd test.dd
00102000: 5241 5350 4933 2020 2020 2008 0000 a37b  RASPI3     ....{
00102010: d452 d452 0000 bb7b d452 0000 0000 0000  .R.R...{.R......
00102020: 412e 0066 0073 0065 0076 000f 00da 6500  A..f.s.e.v....e.
00102030: 6e00 7400 7300 6400 0000 0000 ffff ffff  n.t.s.d.........
00102040: 4653 4556 454e 7e31 2020 2012 008e b57b  FSEVEN~1   ....{
00102050: d452 d452 0000 b57b d452 0300 0000 0000  .R.R...{.R......
00102060: 424f 4f54 434f 4445 4249 4e20 188c bb7b  BOOTCODEBIN ...{
00102070: d452 d452 0000 bb7b d452 0500 e8cc 0000  .R.R...{.R......
00102080: 426e 0000 00ff ffff ffff ff0f 00ca ffff  Bn..............
00102090: ffff ffff ffff ffff ffff 0000 ffff ffff  ................
001020a0: 012e 005f 0062 006f 006f 000f 00ca 7400  ..._.b.o.o....t.
001020b0: 6300 6f00 6400 6500 2e00 0000 6200 6900  c.o.d.e.....b.i.
001020c0: 5f42 4f4f 5443 7e31 4249 4e22 008c bb7b  _BOOTC~1BIN"...{
001020d0: d452 d452 0000 bb7b d452 3900 0010 0000  .R.R...{.R9.....
001020e0: 4649 5855 5020 2020 4441 5420 188c bb7b  FIXUP   DAT ...{
001020f0: d452 d452 0000 bb7b d452 3d00 8e1c 0000  .R.R...{.R=.....
00102100: 412e 005f 0066 0069 0078 000f 004d 7500  A.._.f.i.x...Mu.
00102110: 7000 2e00 6400 6100 7400 0000 0000 ffff  p...d.a.t.......
00102120: 5f46 4958 557e 3120 4441 5422 008c bb7b  _FIXU~1 DAT"...{
00102130: d452 d452 0000 bb7b d452 4500 0010 0000  .R.R...{.RE.....
00102140: 4b45 524e 454c 3820 494d 4720 188c bb7b  KERNEL8 IMG ...{
00102150: d452 d452 0000 bb7b d452 4900 0c1c 0000  .R.R...{.RI.....
00102160: 5354 4152 5420 2020 454c 4620 188c bb7b  START   ELF ...{
00102170: d452 d452 0000 bb7b d452 5100 201d 2d00  .R.R...{.RQ. .-.
00102180: 412e 005f 0073 0074 0061 000f 00bd 7200  A.._.s.t.a....r.
00102190: 7400 2e00 6500 6c00 6600 0000 0000 ffff  t...e.l.f.......
001021a0: 5f53 5441 527e 3120 454c 4622 008d bb7b  _STAR~1 ELF"...{
001021b0: d452 d452 0000 bb7b d452 990b 0010 0000  .R.R...{.R......
```

## 参考

- [Wiki: マスターブートレコード](https://ja.wikipedia.org/wiki/%E3%83%9E%E3%82%B9%E3%82%BF%E3%83%BC%E3%83%96%E3%83%BC%E3%83%88%E3%83%AC%E3%82%B3%E3%83%BC%E3%83%89)
