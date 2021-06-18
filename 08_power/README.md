# チュートリアル 08 - 電源管理

組み込みシステムにとって消費電力は非常に重要です。Raspberry Pi 3は
非常に洗練された電源管理インタフェースを持っています。各デバイスを
個別にオン/オフできます。ただし、これには罠があります。GPIO VCCピンは
ハードワイヤリングされているのでプログラムでオフにすることができません。
つまり、GPIO VCCピンに何らかのデバイスを接続する場合、それらのデバイスを
オフにする方法を実装する必要があります（たとえば、データGPIOピンにトラン
ジスタを接続するなど）。

## power.h, power.c

電源管理コントローラはqemuで正しくエミュレートされないペリフェラルの一つです。
しかし、実際のハードウェアでは動作します。

`power_off()`はボードをシャットダウンして消費電力をほぼゼロにします。

`reset()`はマシンを再起動します。これもPMCが処理します。Raspberry Piには
ハードウェアリセットボタンがないので、非常に便利です。

## main

簡単なメニューを表示してユーザの入力を待ちます。入力に応じて、システムの再起動、
または、電源切断をします。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c delays.c -o delays.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c power.c -o power.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o delays.o main.o mbox.o power.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
 1 - power off
 2 - reset
Choose one: 1

 1 - power off
 2 - reset
Choose one: 2

 1 - power off
 2 - reset
Choose one:
```

**注** 記述どおり、qemuではpower offもresetも効かない。
