# チュートリアル 01 - ベアミニマム

さて、ここでは何もせず、ツールチェーンをテストするだけです。作成されたkernel8.imgは、
Raspberry Piで起動し、CPUコアを無限ループで停止させるはずです。これを確認するには、
次のように実行します。

```sh
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -d in_asm
        ... output removed for clearity, last line: ...
0x0000000000080004:  17ffffff      b #-0x4 (addr 0x80000)
```

## スタート

制御がkernel8.imgに渡されると、C言語の環境は整っていません。そのため、アセンブリで小さな
プリアンブルを実装する必要があります。この最初のチュートリアルは非常にシンプルなので、
今のところC言語は使わず、これだけです。

CPUには4つのコアがあります。今のところ、すべてのコアが同じ無限ループを実行しています。

## Makefile

Makefileはとてもシンプルです。start.Sをコンパイルします。唯一のソースだからです。次に、
リンカフェーズでは、linker.ldスクリプトを使ってリンクします。最後に、出来上がったelf実行
ファイルをrawイメージに変換します。

## リンカスクリプト

驚くことではありませんが、これもシンプルです。kernel8.imgをロードするベースアドレスを設定し、
唯一のセクションを配置するだけです。重要なことは、AArch64の場合、ロードアドレスは**0x80000**であり、
AArch32の場合の**0x8000**ではないことです。

## 実行結果

```
$ make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -d in_asm
----------------
IN:
0x00000300:  d2801b05  mov      x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3              // x6 = core id
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]    // core 1: x4 = 0xd8 + (1<<3) = 0xe0
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x00000300:  d2801b05  mov      x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]    // core 2: x4 = 0xd8 + (2<<3) = 0xe8
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x00000300:  d2801b05  mov      x5, #0xd8
0x00000304:  d53800a6  mrs      x6, mpidr_el1
0x00000308:  924004c6  and      x6, x6, #3
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]    // core 3: x4 = 0xd8 + (3<<3) = 0xf0
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x0000030c:  d503205f  wfe
0x00000310:  f86678a4  ldr      x4, [x5, x6, lsl #3]    // core 0: x4 = 0xd8 + (0<<3) = 0xd8
0x00000314:  b4ffffc4  cbz      x4, #0x30c

----------------
IN:
0x00000000:  580000c0  ldr      x0, #0x18
0x00000004:  aa1f03e1  mov      x1, xzr
0x00000008:  aa1f03e2  mov      x2, xzr
0x0000000c:  aa1f03e3  mov      x3, xzr
0x00000010:  58000084  ldr      x4, #0x20
0x00000014:  d61f0080  br       x4
q
----------------
IN:
0x00080000:  d503205f  wfe
0x00080004:  17ffffff  b        #0x80000

^Cqemu-system-aarch64: terminating on signal 2 from pid 999 (<unknown process>)
```
