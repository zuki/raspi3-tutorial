# チュートリアル 07 - 遅延

低レベルなハードウェアとのインターフェースにおいて、正確な時間だけ
待つことは非常に重要です。このチュートリアルではいくつかの方法を
説明します。1つはCPUの周波数に依存する方法（待ち時間がCPUのクロック
サイクルで与えられる場合に便利です）、他の2つはマイクロ秒（100万分の1秒）
ベースの方法です。

## delays.h, delays.c

`wait_cycles(n)` これは非常に単純なもので`nop`命令をn回実行します。

`wait_msec(n)` この実装はARMシステムレジスタを使用します（すべての
AArch64 CPUで使用可能）。

`wait_msec_st(n)` はBCM固有の実装で、システムタイマペリフェラルを
使用します (qemuでは使用できません: 現行のqemuでは可能)。

## main

シリアルコンソールに文字列を出力する間にさまざまなwait実装を使用します。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c delays.c -o delays.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o delays.o main.o mbox.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Waiting 1000000 CPU cycles (ARM CPU): OK
Waiting 1000000 microsec (ARM CPU): OK
Waiting 1000000 microsec (BCM System Timer): OK
```

**注** 現行のQEMUはSystem Timerも使用できるようだ。
