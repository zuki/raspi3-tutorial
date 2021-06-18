# チュートリアル 05 - UART0, PL011

このチュートリアルでは、チュートリアル04と同じことを行いますが、UART0を使って
シリアル番号を表示します。そのため、qemuでは次のように簡単に実行できます。

```sh
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
My serial number is: 0000000000000000
```

## uart.h, uart.c

レート除算値を使用する前に、PL011用の正確なクロックレートを確立する必要があります。
これは前回と同じプロパティチャネルを使用して、メールボックス経由で行います。それ以外は
このインタフェースはUART1と同じです。

## main

ボードのシリアル番号を問い合わせて、シリアルコンソールに表示します。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o main.o mbox.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
My serial number is: 0000000000000000
```

## 参考サイト

- [Mailbox property interface](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)
