# チュートリアル 04 - メールボックス

UART0を使用するにはメールボックスが必要です。そこで、このチュートリアルではメールボックス
インタフェースを紹介します。これを使ってボードのシリアル番号を照会し、UART1で出力します。

注: qemuはデフォルトではUART1をターミナルにリダイレクトしません。リダイレクトするのは UART0だけです。
そのため、`-serial null -serial stdio`を使用する必要があります。

## uart.h, uart.c

`uart_hex(d)`はバイナリ値を16進形式で表示します。

## mbox.h, mbox.c

メールボックスインタフェースです。まず`mbox`配列にメッセージを詰め込み、次に、
メールボックスチャネルを指定して`mbox_call(ch)`を呼び出し、メッセージを
GPUに渡します。この例では[プロパティチャネル](https://github.com/raspberrypi/firmware/wiki/Mailbox-property-interface)
を使用しています。メッセージは次のようにフォーマットする必要があります。

```
 0. メッセージのバイト単位のサイズ: (x+1)*4
 1. リクエストメッセージを示す既定値: MBOX_REQUEST
 2-x. タグ
 x+1. これ以上タグがないことを示す既定値: MBOX_TAG_LAST
 ```

ここで、タグは次のような形式です。

```
 n+0. タグ識別子
 n+1. 値バッファのバイト単位のサイズ
 n+2. 必ずゼロ
 n+3. オプションの値バッファ
```

## main

ボードのシリアル番号を照会し、シリアルコンソールに表示します。

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
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio
My serial number is: 0000000000000000
```

### タグ識別子: MBOX_TAG_GETMODELの場合

次の値が得られるはずだが、

1. u32: base address in bytes
2. u32: size in bytes

```
$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial null -serial stdio
My memory is: 0000000000000000
```

__要調査__ QEMUではmailboxは動くが取得した値はすべてゼロなのか?
