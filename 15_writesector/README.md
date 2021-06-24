# チュートリアル15 - セクターの書き込み

少しチュートリアル0Bに戻って、SDカードを書き込む機能を追加してみましょう。
これをテストするために、ブートカウンタを実装します。セクタをメモリに読み込んで
（0Bと同じ）、セクタバッファのカウンタを増加させ、SDカードに書き戻します。
こうすることで、このイメージを再起動するたびにカウンタが増加するはずです。

カウンタには、第2セクタの最後の4バイトを選びました。第1セクタは使いたく
ありませんでした。マスタブートレコードが壊れてカードが起動できなくなる
可能性があるからです。第2セクタの方が安全ですが、EFI Partitioning Tableを
使用している場合（私もそうです）は、カウンタがそれを壊してしまう可能性が
あります。そこで、テーブルが508バイトより短いことを期待して、セクタの最後の
4バイトを選びました。もしそうでなければ、COUNTER_SECTOR定義を変更して、
SDカード上の確実に未使用のセクタを指すようにしてください。

徹底的にテストし、コマンド定義中の誤った定数を見つけてくれた[@DamianOslebo](https://github.com/DamianOslebo)に
感謝したいと思います。

## sd.h, sd.c

新しいコマンド（CMD_WRITE_SINGLE, CMD_WRITE_MULTI）と新しいステータスフラグ
（SR_WRITE_AVAILABLE, INT_WRITE_RDY）をドライバに追加しました。これらを*READ*
に対応するものの代わりに使用します。

`sd_writeblock(buffer,lba,num)`は、SDカードのセクタlbaに、バッファからnumブロック
（セクタ）を書き込みます。

## main

メモリのbssセグメントの後のブロックを読み、その中のカウンタを増分し、カードに
保存します。すべてがうまくいけば、実際のカウンタの値をコンソールに表示します。

## 実行結果

```
 make
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
sd_readblock lba 00000001 num 00000001
EMMC: Sending command 11220010 arg 00000200
sd_writeblock lba 00000001 num 00000001
EMMC: Sending command 18220000 arg 00000200
Boot counter AA550001 written to SD card.

$ make run
Boot counter AA550002 written to SD card.

$ make run
Boot counter AA550003 written to SD card.

$ make run
Boot counter AA550004 written to SD card.
```

## メモ

手元の環境では、第2セクタはMBRのコピーで、問題なし。
