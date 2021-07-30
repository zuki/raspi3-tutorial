# Raspberry Pi 3を使ったベアメタルプログラミング

こんにちは、皆さん。このチュートリアルシリーズは、Raspberry Piで
独自のベアメタルアプリケーションをコンパイルしたい人に向けて
作られています。

対象読者はこのハードウェアの初心者であるホピーOS開発者です。
このシリーズでは、シリアルコンソールへの書き込み、コンソールでのキー
入力の読み込み、画面解像度の設定、リニアフレームバッファへの描画など、
基本的なことを行う方法を紹介します。また、ハードウェアのシリアル番号や
ハードウェアにより生成される乱数を取得する方法、ブートパーティションから
ファイルを読み込む方法なども紹介します。

このチュートリアルはOSの書き方を示すでは*ありません*。メモリ管理や仮想
ファイルシステム、マルチタスクの実装方法などのトピックは取り上げません。
Raspberry Piで独自のOSを書こうと思っている方は、このチュートリアルに取り組む
前に調査しておくことを勧めます。このチュートリアルはハードウェアとの相互
作用に重点を置いたものであり、OS理論に関するものではありません。OS理論に
関しては[raspberry-pi-os](https://github.com/s-matyukevich/raspberry-pi-os)を
勧めます。

プログラムのコンパイル方法、ディスクやファイルシステムのイメージの作成
方法など、GNU/Linuxに関する知識は十分にあることを想定しています。この
アーキテクチャ用のクロスコンパイラの設定方法についてのヒントは提供しますが、
それらについては詳しく説明しません。

## なぜ、Raspberry Pi 3なのか

私がこのボードを選んだ理由はいくつかありますが、まず第一に、安価で入手
しやすいことです。第二に、64ビットのマシンであることです。私は32ビットの
プログラミングを随分前にやめました。64ビットの方がはるかにおもしろからです。
なぜなら、アドレス空間が非常に大きく、ストレージ容量よりも大きいので新しい
面白いソリューションを使用できるからです。第三に、MMIOしか使わないので、
プログラムが簡単なことです。

32ビットのチュートリアルとしては以下を勧めます。

[ケンブリッジ大学のチュートリアル](http://www.cl.cam.ac.uk/projects/raspberrypi/tutorials/os/)（ASMと32ビットだけ）、

[David Welch'のチュートリアル](https://github.com/dwelch67/raspberrypi)（ほとんどがCで、64ビットの例が少しあり）、

[Peter Lemon's tutorials](https://github.com/PeterLemon/RaspberryPi)（ASMのみ、64ビットもあり）、

[Leon de Boerのチュートリアル](https://github.com/LdB-ECM/Raspberry-Pi)（CとASM、64ビットもあり、USBやOpenGLなどの複雑な例もあり）

Raspberry Pi 4についてはどうか

同じペリフェラルを持つほぼ同じハードウェアであり、MMIO_BASEアドレスが異なる
だけです（このチュートリアルに関する限り、その違いのほとんどは関係ありません）。RPi4のチュートリアルとしては、以下を勧めす。

[rpi4-osdevチュートリアル](https://isometimes.github.io/rpi4-osdev)

なぜC++ではないのか

「フリースタンディング」モードのC言語ではハードウェアに対して直接開発を行う
ことができます。C++ではこれができません。ランタイムライブラリが必要だからです。
興味のある方は、素晴らしい[Circle C++](https://github.com/rsta2/circle)
ライブラリを見てみることを勧めます。このライブラリには必須のC++ランタイム
だけでなく、このチュートリアルで説明しているRaspberry Piのすべての機能（と
それ以上の機能）が実装されています。

なぜRustではないのか

単なる私の個人的な意見ですが、RustはC++同様、ベアメタルには向かない超高レベルな
言語だからです。しかし、必要なランタイムライブラリを提供すれば、それは可能です。
私が開発したマルチプラットフォームのシステムブートローダには[Rustカーネルの例](https://gitlab.com/bztsrc/bootboot)も
含まれており、@andre-richterはこのチュートリアルをRustに移植しています。彼の
[Rust repository](https://github.com/rust-embedded/rust-raspi3-OS-tutorials)
には非常に多くのコードが追加されているので、この言語に興味があるのであれば
非常に良いスタートラインとなるでしょう。

この件に関して[ycombinator](https://news.ycombinator.com/item?id=24637129)で
いくつか質問が出ました。第一に、「私の個人的な意見」で文章を始めていることに
注意してください（これは、RustはCよりも非常に大きくて複雑な文法を持っており、
実際には、cargoを使ってすべてのライブラリの依存関係をコンパイルしなければ
ならないことを忘れがちだということを意味します）。第二に、誤解しないでいただき
たいのですが、「Rust port」リンクをクリックすることがあまりにも複雑だと感じる
のであれば、おそらく、低レベルのプログラミングはあなたにとってベストな趣味では
ありません。

## 前提条件

チュートリアルを始めるには、クロスコンパイラ（詳細は00_crosscompilerディレクトリ
を参照）と、FATファイルシステムに[ファームウェアファイル](https://github.com/raspberrypi/firmware/tree/master/boot)を
保存したマイクロSDカードが必要です。

各ディレクトリにはMakefile.gccとMakefile.clangがあります。Makefileのシンボリック
リンクがあなたが選択したクロスコンパイラのバージョンを指していることを確認して
ください。このチュートリアルを初めてClangでテストしてくれた[@laroche](https://github.com/laroche)に感謝したいと思います。

[マイクロSDカードのUSBアダプタ](http://media.kingston.com/images/products/prodReader-FCR-MRG2-img.jpg)を
入手することを勧めます（多くのメーカーはSDカードにこのようなアダプタを付けて
売っています）。これがあればUSBメモリのようにデスクトップコンピュータに
カードを接続することができ、特別なカードリーダインターフェイスが不要になり
ます（最近では多くのラップトップに搭載されていますが）。`dd`コマンドが嫌いで
あれば、イメージの書き込みには[USBImager](https://gitlab.com/bztsrc/usbimager)を
勧めます。これは、Windows、MacOSX、Linuxで利用可能なポータブルな実行ファイルを
備えたシンプルなGUIアプリケーションです。

SDカードにMBRパーティションを作成し、LBA FAT32（タイプ0x0C）パーティションを
作成・フォーマットし、`bootcode.bin`、`start.elf`、`fixup.dat`をコピーします。
あるいは、raspbianのイメージをダウンロードしてSDカードに`dd`し、マウントして
不要な.imgファイルを削除するという方法もあります。好きな方を選んでください。
重要なことは、このチュートリアルで作成したkernel8.imgをSDカードのルート
ディレクトリにコピーする必要があり、他に`.img`ファイルが存在しないようにする
ことです。

[USBシリアルでバッグケーブル](https://www.adafruit.com/product/954)を入手する
ことも勧めます。これをGPIOピン14/15に接続して、デスクトップコンピュータで
`minicom`を次のように実行します。

```sh
minicom -b 115200 -D /dev/ttyUSB0
```

## エミュレーション

残念ながら公式のqemuバイナリはまだRaspberry Pi 3をサポートしていません。
しかし、良いニュースがあります、私が実装したので公式にもすぐに来るでしょう
（更新: [qemu 2.12](https://wiki.qemu.org/ChangeLog/2.12#ARM)で利用可能)。
それまでは、最新のソースからqemuをコンパイルする必要があります。コンパイル
したら、次のようにして使うことができます。

```sh
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -serial stdio
```

あるいは（ファイルシステムチュートリアルでは）

```sh
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -drive file=$(yourimagefile),if=sd,format=raw -serial stdio
```

**-M raspi3**
第一引数はRaspberry Pi 3をエミュレートするようqemuに伝えます。

**-kernel kernel8.img**
第2引数は使用するカーネルのファイル名を伝えます。

**-drive file=$(yourimagefile),if=sd,format=raw**
二番目のケースのこの引数はSDカードイメージを伝えます。これは標準的なrasbian
イメージでも構いません。

**-serial stdio**

**-serial null -serial stdio**
最後の引数は、エミュレートされたUART0をqemuを実行しているターミナルの標準
入出力にリダイレクトするもので、シリアルラインに送られたものがすべて表示され、
ターミナルで入力されたキーがすべてvmで受信されます。これはチュートリアル05
以降でのみ動作します。UART1はデフォルトではリダイレクト*されない*からです。
そのため、`-chardev socket,host=localhost,port=1111,id=aux -serial chardev:Aux`
（[@godmar](https://github.com/godmar) さんの情報に感謝します）のような引数を
追加するか、単純に2つの`-serial`引数を使用する（[@cirosantilli](https://github.com/cirosantilli)さんの情報に感謝します）必要があります。

**!!!警告!!!** Qemu のエミュレーションは初歩的なもので、最も一般的なペリフェラル
しかエミュレートされていません。**!!!W警告!!!**

ハードウェアについて

Raspberry Pi 3のハードウェアについては、インターネット上にたくさんのページがあり、詳しく説明されていますので、ここでは簡単に、基本的なことだけを説明します。

このボードには[BCM2837 SoC](https://github.com/raspberrypi/documentation/tree/master/hardware/raspberrypi/bcm2837)チップが搭載されています。このチップには
次のものが含まれます。

 - VideoCore GPU
 - ARM-Cortex-A53 CPU (ARMv8)
 - いくつかのMMIOマップドペリフェラル

興味深いことに、CPUはこのボードのメインプロセッサではありません。
電源を入れると、まずGPUが動作します。GPUはbootcode.binのコードを実行して
初期化を行い、次にstart.elfという実行ファイルをロードして実行します。
これはARM用の実行ファイルではなく、GPU用にコンパイルされたものです。
私たちにとって興味深いことは、start.elfは様々なARM実行ファイルを探す
ことです。このファイルはすべて`kernel`で始まり、`.img`で終わるものです。
私たちはAArch64モードのCPUをプログラムするので、必要なファイルは
`kernel8.img`だけであり、このファイルは最後に探されます。このファイルを
ロードすると、GPUはARMプロセッサのリセットラインをトリガし、これにより
アドレス0x80000（正確には0ですが、GPUが最初に[ARMの初期化とジャンプコード](https://github.com/raspberrypi/tools/blob/master/armstubs/armstub8.S)を
そこに置くので）のコードの実行を開始します。

RAM（Raspberry Pi 3では1G）はCPUとGPUで共有されているので、一方がメモリに
書き込んだ内容を他方が読み取ることができます。混乱を避けるために[mailbox インタフェース](https://github.com/raspberrypi/firmware/wiki/Mailboxes)と
呼ばれる明確な定義が設けられています。CPUはメールボックスにメッセージを
書き込み、GPUにそれを読むよう指示します。GPUは（メッセージがすべてメモリ上に
あることを知っているので）それを解釈し、同じアドレスに応答メッセージを
置きます。CPUはGPUの処理が終了したことを知るためにメモリをポーリングする
必要があります。GPUの処理が終了したらその応答メッセージを読むことができます。

同様に、すべてのペリフェラルはメモリ上でCPUと通信します。各ペリフェラルは
0x3F000000から始まる専用のメモリアドレスを持っていますが、それは実際のRAMの
アドレスではありません（メモリマップドIOと呼ばれます）。現在、ペリフェラルには
メールボックスはなく、各デバイスが独自のプロトコルを持っています。これらの
デバイスに共通しているのは、メモリの読み書きは4バイトアラインのアドレス
（ワードと呼ばれる）を使って32ビット単位で行う必要があり、それぞれに
コントロール/ステータスワードとデータワードがあることです。残念ながら（SoC
チップのメーカーである）Broadcom社は自社製品の文書化がひどいことで有名です。
入手可能な最も良い資料はBCM2835のドキュメントであり、BCM2837でもほぼ同じです。

また、CPUにはメモリ管理ユニットも搭載されており、仮想アドレス空間を作成する
ことができます。これは、特定のCPUレジスタによってプログラムすることができ、
これらのMMIOアドレスを仮想アドレス空間にマッピングする際には注意が必要です。

興味深いMMIOアドレスをいくつか紹介します。
```
0x3F003000 - システムタイマ
0x3F00B000 - 割り込みコントローラ
0x3F00B880 - VideoCoreメールボックス
0x3F100000 - 電源管理
0x3F104000 - 乱数生成器
0x3F200000 - GPIOコントローラ
0x3F201000 - UART0 (シリアルポート, PL011)
0x3F215000 - UART1 (シリアルポート, AUX mini UART)
0x3F300000 - 外部マスメディアコントローラ（EMMC, SDカードリーダ）
0x3F980000 - USBコントローラ
```

より詳しい情報はRaspberry Pi githubのwikiとドキュメントを参照してください。

https://github.com/raspberrypi

頑張って、ラズベリーでハッキングを楽しんでください :-)

bzt

# チュートリアル一覧

- [00: クロスコンパイラ](00_crosscompiler/README.md)
- [01: ベアメタル](01_bareminimum/README.md)
- [02: マルチコア](02_multicorec/README.md)
- [03: Uart1](03_uart1/README.md)
- [04: メールボックス](04_mailboxes/README.md)
- [05: Uart0](05_uart0/README.md)
- [06: ランダム](06_random/README.md)
- [07: 遅延](07_delays/README.md)
- [08: 電源](08_power/README.md)
- [09: フレームバッファ](09_framebuffer/README.md)
- [0A: スクリーンフォント](oA_pcscreenfont/README.md)
- [0B: セクタの読み込み](0B_readsector/README.md)
- [0C: ディレクトリ](0C_directory/README.md)
- [0D: ファイルの読み込み](0D_readfile/README.md)
- [0E: initrd](0E_initrd/README.md)
- [0F: 実行レベル](0F_executionlevel/README.md)
- [10: 仮想メモリ](10_virtualmemory/README.md)
- [11: 例外](11_exceptions/README.md)
- [12: printf](12_printf/README.md)
- [13: デバッガ](13_debugger/README.md)
- [14: raspbootin64](14_raspbootin64/README.md)
- [15: セクタの書き込み](15_writesector/README.md)
