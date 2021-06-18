# チュートリアル 03 - UART1, mini UART

有名な"Hello World"の時間です。まずUART1について説明します。UART1はクロック周波数が
固定されており、プログラムしやすいからです。 UART1

注: qemuはデフォルトではUART1をターミナルにリダイレクトしません。リダイレクトするのは
UART0だけです。そのため、`-serial null -serial stdio`を使用する必要があります。

## gpio.h

新たにヘッダファイルを用意しました。このファイルはMMIOのベースアドレスとGPIOコントローラの
アドレスを定義しています。このファイルは多くのデバイスが必要とするため非常に人気のある
ファイルになるでしょう。

## uart.h, uart.c

必要最小限の実装です。

`uart_init()`はデバイスを初期化し、GPIOポートにマッピングします。

`uart_send(c)`はシリアルラインを通じて文字を送信します。

`uart_getc()`は文字を受信します。復帰文字(13)は改行文字(10)に変換されます。

`uart_puts(s)`は文字列をプリントアウトします。改行文字については復帰文字も
送信されます（13 + 10）。

## main

まず、uartの初期化コードを呼び出す必要があります。次いで"Hello World!"を返します。
USBシリアルケーブルを購入していれば、minicomの画面に表示されるはずです。その後は
minicomに入力されたすべての文字がエコーバックされます。ローカルエコーをオフにして
いないと、押したキーを2回見ることになります。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o main.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio
Hello World!
abcdf qemu-system-aarch64: terminating on signal 2 from pid 999 (<unknown process>)
```
