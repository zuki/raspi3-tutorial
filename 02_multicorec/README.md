# チュートリアル02 - マルチコアC

今回はもっと複雑なことをやってみましょうか。複雑とは、最初のチュートリアルのように
CPUコアを停止させることですが、今回はそのうちの1つをC言語で停止させます。

## start

今回はコアを識別する必要があります。そのために*mpidr_el1*システムレジスタを読みます。
それが0でなければ、前回と同じく無限ループを行います。0ならC関数を呼び出します。
そのためには、call命令をする前に適切なスタックが必要であり、メモリのbssセグメントを
ゼロクリアする必要があります。それらを行うコードをアセンブリに追加しました。Cコードが
リターンした場合（しないはず）に備えて、他のCPUコアが実行しているのものと同じ無限ループに
ジャンプします。

注: ファームウェアのバージョンによっては、アプリケーションコアが停止している場合があります。
その場合、私たちのコードはコア0でしか動作しませんが、コア番号を確認しても害はありません。
他のコアを起動するには、実行する関数のアドレスを0xE0、0xE8、0xF0（この順でコアごとに1アドレス）に
書き込む必要があります。[armstub8.S](https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S#L129)を
参照してください。

## Makefile

ちょっとトリッキーなものになりました。Cソースをコンパイルするためのコマンドを追加しましたが、
それは規則に従った方法にしました。これにより、今後はCソースの数にかかわらず、すべてのチュートリアルで
同じMakefileを使用することができますので、これ以上の議論はしません。

## リンカスクリプト

同様に、リンカスクリプトも複雑になりました。C言語にはdataとbssセクションが必要だからです。
bssのサイズの計算も追加し、アセンブリコードから手間をかけずに参照できるようにしました。

アセンブリコードではテキストセグメントの開始アドレスが重要です。なぜなら、その直前にスタックを
設定するからです。そのためにKEEP()を使用しています。この指定によりロードアドレスは0x80000であり、
`_start`ラベルもスタックトップも同じアドレスとなります。

## main

最後に、私たちの最初のCコードです。ただの空ループですが、それでも。:-)

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o main.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -d in_asm
----------------
IN:
0x00000300:  d2801b05  mov      x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x00000300:  d2801b05  mov      x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x00000300:  d2801b05  mov      x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x00000000:  580000c0  ldr      x0, #0x18
0x00000004:  aa1f03e1  mov      x1, xzr
0x00000008:  aa1f03e2  mov      x2, xzr
0x0000000c:  aa1f03e3  mov      x3, xzr
0x00000010:  58000084  ldr      x4, #0x20
0x00000014:  d61f0080  br       x4

----------------
IN:
0x00080000:  d53800a1  mrs      x1, mpidr_el1
0x00080004:  92400421  and      x1, x1, #3
0x00080008:  b4000061  cbz      x1, #0x80014

----------------
IN:
0x00080014:  58000161  ldr      x1, #0x80040
0x00080018:  9100003f  mov      sp, x1
0x0008001c:  58000161  ldr      x1, #0x80048
0x00080020:  180000e2  ldr      w2, #0x8003c
0x00080024:  34000082  cbz      w2, #0x80034

----------------
IN:
0x00080034:  94000007  bl       #0x80050

----------------
IN:
0x00080050:  14000000  b        #0x80050

^Cqemu-system-aarch64: terminating on signal 2 from pid 999 (<unknown process>)
```
