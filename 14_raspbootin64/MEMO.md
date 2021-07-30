# メモ

  - `rust_raspi_os/06_uart_chainloader`と同じプロトコルを使用している
  - このイメージ自体はSDカードに書いておいて（0x80000 - 1024に書かれる）、
    以後、作成されたイメージはraspbootcom経由（あるいは`minipush.rb`経由）で
    送信する。

# 実行結果

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

## QEMUのserialを送信機にどうつなぐかが、今のところ不明



- qemuのシリアルをptyに接続

```
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -chardev pty,id=pty0 -serial chardev:pty0
char device redirected to /dev/ttys003 (label pty0)
```

### 1. aspbootcomを使用

- 送信している様子はない。

```
$ ./raspbootcom /dev/ttys003 ../../raspi3-tutorial/13_debugger/kernel8.img
Raspbootcom V1.0
### Listening on /dev/ttys003
```

### 2. USBImagerを使用

- 画面でデバイスを選択できない。
- コマンドラインで指定してもだめ。
- そもそもラーが出ているが。[stack overflow](https://stackoverflow.com/questions/46999695/class-fifindersyncextensionhost-is-implemented-in-both-warning-in-xcode-si)に
  解決策と思われる記事があるがpythonのよう。要調査。

```
$ ./usbimager -S /dev/ttys003
2021-06-23 15:07:40.702 usbimager[7274:63164] get 0x0
objc[7274]: Class FIFinderSyncExtensionHost is implemented in both /System/Library/PrivateFrameworks/FinderKit.framework/Versions/A/FinderKit (0x7fffa52143f0) and /System/Library/PrivateFrameworks/FileProvider.framework/OverrideBundles/FinderSyncCollaborationFileProviderOverride.bundle/Contents/MacOS/FinderSyncCollaborationFileProviderOverride (0x8fd3f50). One of the two will be used. Which one is undefined.
```

## [issue#53](https://github.com/bztsrc/raspi3-tutorial/issues/53)が該当すると
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

## 現状の問題点

- qemuを実行するとそのターミナルの/dev/ttysxxxもオープンする。
- したがって、rasbootcomでそのdevを指定すれば良いように思える。
- 問題は両者の実行タイミングで、qemuを先に実行するとbootcomを実行した時には
  すでに'\c\c\c'は送信済みで受け取ることができない。
- bootcomを先に実行しようとすると`error on device`でコンソールが落ちてしまう。

### 対策1: qemuで実行後数秒待ってから送信を行う。その間にbootcomを立ち上げる

結果は同じで、bootcomは受信しない。

### 対策2: bootcomから'READY’を送り、qemuはこれを受け取ってから送信を行う。

qemuにデータは届いたが、プログラムにデータが渡らない。

```
$ ./raspbootcom /dev/ttys000 ../../raspi3-tutorial/13_debugger/kernel8.img
Raspbootcom V1.0 - BOOTBOOT version

### Listening on /dev/ttys000
write RD
```

```
$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
RD
```

# 実機による実行

1. 14_raspbootin64のカーネルをSDカードにコピー
   ```
   $ cp 14_raspbootin64/kernel8.img /Volume/boot/
   ```
2. `utils/minipush.rb`を実行
3. raspiの電源オン

```
$ ruby utils/minipush.rb /dev/cu.usbserial-AI057C9L ~/raspi_os/raspi3-tutorial/04_mailboxes/kernel8.img

Minipush 1.0

/Users/dspace/.rbenv/versions/2.7.1/lib/ruby/gems/2.7.0/gems/serialport-1.3.1/lib/serialport.rb:25: warning: rb_secure will be removed in Ruby 3.0
[MP] ✅ Serial connected
[MP] 🔌 Please power the target now   // ここでraspiの電源オン
RBIN64                                // 14_raspbootin64のカーネルが処理開始
[MP] ⏩ Pushing 0 KiB 🦀                                             0% 0 KiB/s
[MP] ⏩ Pushing 0 KiB ==========================================🦀 100% 0 KiB/s Time: 00:00:00
My serial number is: 000000004AABE848 // 4_mailboxesのカーネルの処理結果

[MP] Bye 👋
```


[READMEに戻る](README.md)
