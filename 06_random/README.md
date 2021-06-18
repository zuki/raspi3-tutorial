# チュートリアル 06 - ハードウェア乱数生成器

このチュートリアルは簡単です。(文書化されていない)ハードウェア乱数
ジェネレータから数値を取得します。これを使えば、どんなゲームでも
簡単で正確なサイコロ投げを実装することができます。ハードウェアの
サポートがないと擬似乱数しか生成できないのでこれは重要です。

## rand.h, rand.c

`rand_init()`はハードウェアを初期化します。

`rand(min,max)`はmin以上、max未満の乱数を返します。

## main

乱数値を問い合わせて、シリアルコンソールに表示します。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c rand.c -o rand.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o main.o mbox.o rand.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img
dspace@mini:~/raspi_os/raspi3-tutorial/06_random$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Here goes a random number: 3A9D4A64
```

### サイコロとして使う

```
for (int i=0; i<10; i++) {
    uart_hex(rand(1,7));
    uart_puts("\n");
}

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
00000005
00000005
00000002
00000003
00000004
00000001
00000003
00000005
00000006
00000006
```
