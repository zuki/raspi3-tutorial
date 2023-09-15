チュートリアル 0E - 初期RAMディスク

多くのOSでは、起動時に初期RAMディスクを使ってファイルをメモリにロードします。
趣味でOSを開発している人のほとんどはこの方法をきちんと学んでいないので、この
ようなチュートリアルが必要だと思いました。

まず第一に、私たちは車輪の再発明をしたり、新しいフォーマットや面倒なイメージ作成
ツールを考えたりするつもりはありません。POSIX標準の`tar`ユーティリティと`cpio`
ユーティリティを使って`initrd`を作成することにします。前者は簡単であり、後者は
Linuxで使用されています。

tarのフォーマットは非常にシンプルです。まず、ファイルのメタ情報を含む512バイトの
ヘッダがあり、次にファイルコンテンツが続きます。コンテンツは512バイトの倍数に
なるようにゼロ詰めされます。アーカイブ内のすべてのファイルに対してこれが繰り返され
ます。

cpioもよく似ていますが、可変長のヘッダを持ち、ファイルコンテンツはゼロ詰めされません。

圧縮されたinitrdが必要な場合は、たとえば[tinf](https://github.com/jibsen/tinf)
ライブラリを使って解凍することができます。圧縮されていないバッファはここで説明する
方法でパースすることができます。

次に、RAMディスクのメモリへの読み込みについてですが、いくつかの選択肢があります。

### 自分でファイルをロードする

前回のチュートリアルで説明した`fat_readfile()`を使うことができます。この場合、
initrdのアドレスはが関数から*返されます*。

### GPUに依頼する

`config.txt`を使い、initrdをロードするよう、start.elfに指示することができます。
これにより、SDカードリーダやFATパーサが一切不要になり、結果的にカーネルが非常に
小さくなります。[config.txt](https://www.raspberrypi.org/documentation/configuration/config-txt/boot.md)に
関しては、2つの選択肢があります。

`ramfsfile=(filename)` - これはカーネルの後に(filename)をロードします。initrdには
リンカスクリプトで定義されているラベル *&_end* でアクセスできます。

`initramfs (filename) (address)` - (filename)を指定した場所にロードします。initrdには
 *(address)* でアクセスできます。

### 静的リンク

これはあまり実用的ではありません。initrdを変更するたびにカーネルをビルドする必要が
あるからです。しかし、これは最もシンプルな方法であり、チュートリアルをシンプルに
するためにこの方法を使用します。initrdにはラベル *_binary_ramdisk_start* でアクセス
できます。

訳注: `ld -r -b type -d file.o file`とすると`_type_file_start`, `_type_file_end`,
`_type_file_size`の3つのシンボルが作成される。

## Makefile

アーカイブをオブジェクトファイルに変換するための`rd.o`ルールを追加しました。
また、以下のターゲットも追加しました。

`make tar`は、*tar*フォーマットのアーカイブを作成します。

`make cpio`は、*cpio hpodc*アーカイブを作成します。

したがって、コンパイルには、`make tar all`か、`make cpio all`を使う必要があります。

## initrd.h, initrd.c

`initrd_list(buf)`は、バッファにあるアーカイブの内容をリストします。フォーマットは
自動検出します。

## main

コンソールを初期化して、initrdバッファをリスナに渡します。

## 実行結果

### 1. tar

```
$ make tar all
tar -cf ramdisk *.md *.c *.h
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-ld -r -b binary -o rd.o ramdisk
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c delays.c -o delays.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c initrd.c -o initrd.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o rd.o delays.o initrd.o main.o mbox.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Type     Offset   Size     Access rights        Filename
regular  00080C60 00000C03 000644  dspace.staff OLVASSEL.md
regular  00081C60 00000E60 000644  dspace.staff README.md
regular  00082E60 00000A3F 000644  dspace.staff README_en.md
regular  00083C60 00000A0C 000644  dspace.staff delays.c
regular  00084A60 00001470 000644  dspace.staff initrd.c
regular  00086260 000005D0 000644  dspace.staff main.c
regular  00086A60 00000A0E 000644  dspace.staff mbox.c
regular  00087860 000011D8 000644  dspace.staff uart.c
regular  00088C60 000004F5 000644  dspace.staff delays.h
regular  00089460 000009B4 000644  dspace.staff gpio.h
regular  0008A060 0000048B 000644  dspace.staff initrd.h
regular  0008A860 0000066A 000644  dspace.staff mbox.h
regular  0008B260 00000505 000644  dspace.staff uart.h

$ xxd ramdisk | head -37
00000000: 4f4c 5641 5353 454c 2e6d 6400 0000 0000  OLVASSEL.md.....
00000010: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000020: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000030: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000040: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000050: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000060: 0000 0000 3030 3036 3434 2000 3030 3037  ....000644 .0007
00000070: 3635 2000 3030 3030 3234 2000 3030 3030  65 .000024 .0000
00000080: 3030 3036 3030 3320 3134 3036 3235 3636  0006003 14062566
00000090: 3033 3420 3031 3332 3337 0020 3000 0000  034 013237. 0...
000000a0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000000b0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000000c0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000000d0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000000e0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000000f0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000100: 0075 7374 6172 0030 3064 7370 6163 6500  .ustar.00dspace.
00000110: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000120: 0000 0000 0000 0000 0073 7461 6666 0000  .........staff..
00000130: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000140: 0000 0000 0000 0000 0030 3030 3030 3020  .........000000
00000150: 0030 3030 3030 3020 0000 0000 0000 0000  .000000 ........
00000160: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000170: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000180: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000190: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001a0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001b0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001c0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001d0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001e0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
000001f0: 0000 0000 0000 0000 0000 0000 0000 0000  ................
00000200: 4f6b 7461 74c3 b361 6e79 6167 2030 4520  Oktat..anyag 0E
00000210: 2d20 4b65 7a64 6574 6920 6d65 6dc3 b372  - Kezdeti mem..r
00000220: 6961 206c 656d 657a 0a3d 3d3d 3d3d 3d3d  ia lemez.=======
00000230: 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d  ================
00000240: 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d 3d0a  ===============.

$ aarch64-none-elf-nm kernel8.elf
000000000008d6f0 B __bss_end
0000000000000012 A __bss_size
000000000008d660 B __bss_start
000000000008d660 D _binary_ramdisk_end
000000000000cc00 A _binary_ramdisk_size
0000000000080a60 D _binary_ramdisk_start
000000000008d6f0 B _end
0000000000080000 T _start
00000000000800c0 T get_system_timer
00000000000801d0 T initrd_list
00000000000804e0 T main
```

### 2. cpio

```
$ make cpio all
ls *.md *.c *.h | cpio -H odc -o >ramdisk
73 blocks
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-ld -r -b binary -o rd.o ramdisk
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c delays.c -o delays.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c initrd.c -o initrd.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o rd.o delays.o initrd.o main.o mbox.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Type     Offset   Size     Access rights        Filename
000081A4 00080AB8 00000C03 000001F5.00000014    OLVASSEL.md
000081A4 00081711 0000164C 000001F5.00000014    README.md
000081A4 00082DB6 00000A3F 000001F5.00000014    README_en.md
000081A4 0008384A 00000A0C 000001F5.00000014    delays.c
000081A4 000842AB 000004F5 000001F5.00000014    delays.h
000081A4 000847F3 000009B4 000001F5.00000014    gpio.h
000081A4 000851FC 00001470 000001F5.00000014    initrd.c
000081A4 000866C1 0000048B 000001F5.00000014    initrd.h
000081A4 00086B9F 000005D0 000001F5.00000014    main.c
000081A4 000871C2 00000A0E 000001F5.00000014    mbox.c
000081A4 00087C23 0000066A 000001F5.00000014    mbox.h
000081A4 000882E0 000011D8 000001F5.00000014    uart.c
000081A4 0008950B 00000505 000001F5.00000014    uart.h

$ xxd ramdisk | head
00000000: 3037 3037 3037 3737 3737 3737 3030 3030  0707077777770000
00000010: 3031 3130 3036 3434 3030 3037 3635 3030  0110064400076500
00000020: 3030 3234 3030 3030 3031 3030 3030 3030  0024000001000000
00000030: 3134 3036 3235 3636 3033 3430 3030 3031  1406256603400001
00000040: 3430 3030 3030 3030 3630 3033 4f4c 5641  400000006003OLVA
00000050: 5353 454c 2e6d 6400 4f6b 7461 74c3 b361  SSEL.md.Oktat..a
00000060: 6e79 6167 2030 4520 2d20 4b65 7a64 6574  nyag 0E - Kezdet
00000070: 6920 6d65 6dc3 b372 6961 206c 656d 657a  i mem..ria lemez
00000080: 0a3d 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d  .===============
00000090: 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d 3d3d  ================

$ aarch64-none-elf-nm kernel8.elf
000000000008b0f0 B __bss_end
0000000000000012 A __bss_size
000000000008b060 B __bss_start
000000000008b060 D _binary_ramdisk_end
000000000000a600 A _binary_ramdisk_size
0000000000080a60 D _binary_ramdisk_start
000000000008b0f0 B _end
0000000000080000 T _start
00000000000800c0 T get_system_timer
00000000000801d0 T initrd_list
00000000000804e0 T main
```

## メモ

- Macではcpioの-Hのformat名が違っており、`-H hpodc`を`-H odc`に変更した。
