# チュートリアル 14 - Raspbootin64

SDカードを交換するのは面倒だし、SD カードの破損を避けるためにも、シリアル
経由で本物のkernel8.imgをロードするkernel8.imgを作成します。

このチュートリアルは、有名なシリアルブートローダである[raspbootin](https://github.com/mrvn/raspbootin)を
64bitで書き直したものです。私が提供するのはローダの半分のRPi上で動作する
カーネル受信機だけです。もう半分のPC上で動作する送信側については、オリジナルの
[raspbootcom](https://github.com/mrvn/raspbootin/blob/master/raspbootcom/raspbootcom.cc)
ユーティリティを参照してください。WindowsやMacOSXからカーネルを送信したい場合や
GUIアプリケーションが好きな場合は[USBImager](https://gitlab.com/bztsrc/usbimager)
アプリケーションに`-S`(大文字のS)フラグを付けて使用することを勧めます。さらに、
[@milanvidakovic](https://github.com/milanvidakovic)は親切にもJavaバージョンの
カーネル送信機のを共有してくれました（名前に惑わされないでください、これは
実際にサーバ部分です、名前は気にしないでください）。

新しいカーネルを同じアドレスにロードするには、自分自身を邪魔にならないように
移動させる必要があります。これはチェインローディングと呼ばれるもので、ある
コードが次のコードをメモリの同じ位置にロードすることです。つまり、後者が
ファームウェアによってロードされると考えるのです。これを実装するために、
これまでとは異なるリンクアドレスを使用します。GPUはお構いなしに0x80000に
ロードするので、私たちはコードをそのリンクアドレスにコピーする必要があります。
コピーが終わったら、0x80000のメモリは自由に使えるようにしなければなりません。

また、ローダのサイズは最小限にする必要があります。なぜなら、それはいずれにせよ
新しくロードされるコードだとみなされるからです。`uart_puts()`などの関数を削除
することで、ローダのサイズを1024バイト以下にすることができました。これにより、
リンクアドレス(0x80000 - 1024)が適切であり、ローダのコードがロードアドレスを
超えないことを保証することができました。これは次のように確認できます。

```sh
$ aarch64-elf-readelf -s kernel8.elf | grep __bss_end
    27: 000000000007ffb0     0 NOTYPE  GLOBAL DEFAULT    4 __bss_end
```

## start

ファームウェアから渡される引数をレジスタに保存する必要があります。本来
ロードされるべきアドレスにコードを再配置するためのループを追加しました。
最後に、gccはRIP相対ジャンブを生成するので、再配置されたCコードにジャンプ
するように分岐命令を調整する必要があります。

再配置されていないスピンループを実行すると非常にまずいことに気付いた
[@mrvn](https://github.com/mrvn)さんに感謝します。ファームウェアの変更により、
このコードはBSP上でのみ実行されるようになったため問題が発生せず、誰も
気づきませんでした。

## linker

今回は異なるリンクアドレスを使用しています。bssのサイズ計算と同じように、
コードのサイズを計算して、コピーが必要なバイト数を把握します。

## main

"RBIN64"と表示し、シリアル経由で新しいカーネルを受け取り、start.elfがロード
されるはずだったメモリアドレスに保存します。それが終わったら、引数を復元し、
絶対アドレスを使って新しいカーネルにジャンプします。

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
dspace@mini:~/raspi_os/raspi3-tutorial/14_raspbootin64$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
RBIN64
```

### QEMUのserialを送信機にどうつなぐかが、今のところ不明



- qemuのシリアルをptyに接続

```
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -chardev pty,id=pty0 -serial chardev:pty0
char device redirected to /dev/ttys003 (label pty0)
```

#### 1. aspbootcomを使用

- 送信している様子はない。

```
$ ./raspbootcom /dev/ttys003 ../../raspi3-tutorial/13_debugger/kernel8.img
Raspbootcom V1.0
### Listening on /dev/ttys003
```

#### 2. USBImagerを使用

- 画面でデバイスを選択できない。
- コマンドラインで指定してもだめ。
- そもそもラーが出ているが。[stack overflow](https://stackoverflow.com/questions/46999695/class-fifindersyncextensionhost-is-implemented-in-both-warning-in-xcode-si)に
  解決策と思われる記事があるがpythonのよう。要調査。

```
$ ./usbimager -S /dev/ttys003
2021-06-23 15:07:40.702 usbimager[7274:63164] get 0x0
objc[7274]: Class FIFinderSyncExtensionHost is implemented in both /System/Library/PrivateFrameworks/FinderKit.framework/Versions/A/FinderKit (0x7fffa52143f0) and /System/Library/PrivateFrameworks/FileProvider.framework/OverrideBundles/FinderSyncCollaborationFileProviderOverride.bundle/Contents/MacOS/FinderSyncCollaborationFileProviderOverride (0x8fd3f50). One of the two will be used. Which one is undefined.
```

### [issue#53](https://github.com/bztsrc/raspi3-tutorial/issues/53)が該当すると
思われるが、以下の通り実行しても結果は同じ。

```
$ make run
```

別ターミナルで、

```
$ ps aux | grep qemu
$ lsof -p qemuプロセスID | grep dev
$ ./raspbootcom /dev/ttys00? ../../raspi3-tutorial/13_debugger/kernel8.img
```
