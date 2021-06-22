# チュートリアル12 - printf

例外ハンドラを改良するにはCライブラリの有名な関数のいくつかが必要に
なります。私たちはベアメタルでプログラムをしているので、libcはありません。
そのため、printf()を自分で実装する必要があります。

```sh
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Hello World!
This is character 'A', a hex number: 7FFF and in decimal: 32767
Padding test: '00007FFF', '    -123'
```

## sprintf.h, sprintf.c

興味深い部分です。ここでは可変長引数リストを扱うためにコンパイラの機能に
大きく依存しています。いつものチュートリアルと同じように、この関数は完全な
機能は持っておらず、最低限の実装になっています。'%s', '%c', '%d',  '%x'を
サポートしています。パディングには制限があり、16進数は先頭ゼロ詰め、
10進数は先頭空白詰めの右揃えのみです。

`sprintf(dst, fmt, ...)`は、printfと同じですが、結果を文字列で保存します。

`vsprintf(dst, fmt, va)`は、可変長の引数リストではなく、引数リストのパラ
メータを受け取るバリアントです。


## uart.h, uart.c

`printf(fmt, ...)`は、古き良きCライブラリ関数です。上のsprintf関数を使用
して、uart_puts()と同じ方法で文字列を出力しています。'%x'をサポートしたので。
uart_hex()は不要となり、削除しました。

## start

私たちは浮動小数点や倍精度整数を使用しませんが，gccの組み込み関数が使用する
可能性があります。そのため、「未定義命令」という例外を回避するためにFPU
コプロセッサを有効にする必要があります。また、適切な例外ハンドラを作って
いないので、今回はダミーの`exc_handler`スタブを用意しました。

## main

printfの実装をテストします。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c sprintf.c -o sprintf.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -fno-stack-protector -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o main.o mbox.o sprintf.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Hello World!
This is character 'A', a hex number: 7FFF and in decimal: 32767
Padding test: '00007FFF', '    -123'
```
