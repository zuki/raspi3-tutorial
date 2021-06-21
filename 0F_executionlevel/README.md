チュートリアル 0F - 実行レベル

仮想メモリを検討する前に、実行レベルについて説明する必要があります。各レベルは
独自のメモリ変換テーブルを持っているので、現在どのテーブルを使用しているのかを
知ることは非常に重要です。今回のチュートリアルでは、スーパバイザレベル（EL1）に
いることを確認します。QemuはEL1で起動できますが、実際のRaspberry Piは通常ハイパー
バイザレベルのEL2で起動します。qemuでは"-d int"を使用してレベル変更のデバッグを
行います。

```sh
$ qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio -d int
Exception return from AArch64 EL2 to AArch64 EL1 PC 0x8004c
Current EL is: 00000001
```

## start

スーパバイザレベルにいない場合に実行レベルを変更するためのアセンブリコードを
少しだけ追加しました。しかし、それを行うには（`wait_msec()`で使用している）
カウンタレジスタへのアクセス権限を取得し、EL1におけるAArch64モードにしたいことを
CPUに伝えなければなりません。最後に、実際にレベルを変更するためにフェイクの例外
リターンを行います。

注: 完全を期すために、[Issue #6](https://github.com/bztsrc/raspi3-tutorial/issues/6)の
ためにEL3に関するコードも追加していますが、通常、Raspberryはkernel8.imgをEL2で実行
します。config.txtのオプションを使うことで、EL3で動作させることもできます（[@btauro](https://github.com/btauro)さんの情報に感謝します）。

## main

現在の実行レベルを問い合わせて、シリアルコンソールに表示します。

## 実行結果

```
$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
Current EL is: 00000001

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio -d int
Exception return from AArch64 EL2 to AArch64 EL1 PC 0x80084
Current EL is: 00000001
```

```
$ aarch64-none-elf-objdump -d kernel8.elf

0000000000080000 <_start>:
   80000:	d53800a1 	mrs	x1, mpidr_el1
   80004:	92400421 	and	x1, x1, #0x3
   80008:	b4000061 	cbz	x1, 80014 <_start+0x14>
   8000c:	d503205f 	wfe
   80010:	17ffffff 	b	8000c <_start+0xc>
   80014:	580004e1 	ldr	x1, 800b0 <_start+0xb0>
   80018:	d5384240 	mrs	x0, currentel
   8001c:	927e0400 	and	x0, x0, #0xc
   80020:	f100301f 	cmp	x0, #0xc
   80024:	54000101 	b.ne	80044 <_start+0x44>  // b.any
   80028:	d280b622 	mov	x2, #0x5b1                 	// #1457
   8002c:	d51e1102 	msr	scr_el3, x2
   80030:	d2807922 	mov	x2, #0x3c9                 	// #969
   80034:	d51e4002 	msr	spsr_el3, x2
   80038:	10000062 	adr	x2, 80044 <_start+0x44>
   8003c:	d51e4022 	msr	elr_el3, x2
   80040:	d69f03e0 	eret
   80044:	f100101f 	cmp	x0, #0x4
   80048:	540001e0 	b.eq	80084 <_start+0x84>  // b.none
   8004c:	d51c4101 	msr	sp_el1, x1
   80050:	d53ce100 	mrs	x0, cnthctl_el2
   80054:	b2400400 	orr	x0, x0, #0x3
   80058:	d51ce100 	msr	cnthctl_el2, x0
   8005c:	d51ce07f 	msr	cntvoff_el2, xzr
   80060:	d2b00000 	mov	x0, #0x80000000            	// #2147483648
   80064:	b27f0000 	orr	x0, x0, #0x2
   80068:	d51c1100 	msr	hcr_el2, x0
   8006c:	d53c1100 	mrs	x0, hcr_el2
   80070:	d2807882 	mov	x2, #0x3c4                 	// #964
   80074:	d51c4002 	msr	spsr_el2, x2
   80078:	10000062 	adr	x2, 80084 <_start+0x84>
   8007c:	d51c4022 	msr	elr_el2, x2
   80080:	d69f03e0 	eret
   80084:	9100003f 	mov	sp, x1
   80088:	58000181 	ldr	x1, 800b8 <_start+0xb8>
   8008c:	180000e2 	ldr	w2, 800a8 <_start+0xa8>
   80090:	34000082 	cbz	w2, 800a0 <_start+0xa0>
   80094:	f800843f 	str	xzr, [x1], #8
   80098:	51000442 	sub	w2, w2, #0x1
   8009c:	35ffffa2 	cbnz	w2, 80090 <_start+0x90>
   800a0:	94000008 	bl	800c0 <main>
   800a4:	17ffffda 	b	8000c <_start+0xc>
```
