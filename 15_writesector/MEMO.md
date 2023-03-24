# メモ: チュートリアル15 - セクターの書き込み

## 実機で実行

```
$ make chainboot

Minipush 1.0

/Users/dspace/.rbenv/versions/2.7.1/lib/ruby/gems/2.7.0/gems/serialport-1.3.1/lib/serialport.rb:25: warning: rb_secure will be removed in Ruby 3.0
[MP] ✅ Serial connected
[MP] 🔌 Please power the target now
RBIN64
[MP] ⏩ Pushing 0 KiB 🦀                                             0% 0 KiB/s
[MP] ⏩ Pushing 3 KiB ======================🦀                      53% 0 KiB/s
[MP] ⏩ Pushing 5 KiB ================================🦀            76% 0 KiB/s
[MP] ⏩ Pushing 6 KiB ==========================================🦀 100% 0 KiB/s Time: 00:00:00
EMMC: GPIO set up
EMMC: reset OK
sd_clk divisor 00000068, shift 00000006
EMMC: Sending command 00000000 arg 00000000
EMMC: Sending command 08020000 arg 000001AA
EMMC: Sending command 37000000 arg 00000000
EMMC: Sending command 29020000 arg 51FF8000
EMMC: CMD_SEND_OP_COND returned VOLTAGE 0000000000F98000
EMMC: Sending command 37000000 arg 00000000
EMMC: Sending command 29020000 arg 51FF8000
EMMC: CMD_SEND_OP_COND returned COMPLETE VOLTAGE CCS FFFFFFFFC1F98000
EMMC: Sending command 02010000 arg 00000000
EMMC: Sending command 03020000 arg 00000000
EMMC: CMD_SEND_REL_ADDR returned 0000000012340000
sd_clk divisor 00000002, shift 00000000
EMMC: Sending command 07030000 arg 12340000
EMMC: Sending command 37020000 arg 12340000
EMMC: Sending command 33220010 arg 00000000
EMMC: Sending command 37020000 arg 12340000
EMMC: Sending command 06020000 arg 12340002
EMMC: supports SET_BLKCNT CCS
sd_readblock lba 00000001 num 00000001
EMMC: Sending command 11220010 arg 00000001
sd_writeblock lba 00000001 num 00000001
EMMC: Sending command 18220000 arg 00000001
Boot counter 00000001 written to SD card.           // <= 00000001

[MP] Bye 👋

$ make chainboot
...
Boot counter 00000002 written to SD card.           // <= 00000002

$ make chainboot
...
Boot counter 00000003 written to SD card.           // <= 00000003
```

## QEMUで実行

```
 make
rm kernel8.elf *.o >/dev/null 2>/dev/null || true
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c start.S -o start.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c delays.c -o delays.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c main.c -o main.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c mbox.c -o mbox.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c sd.c -o sd.o
aarch64-none-elf-gcc -Wall -O2 -ffreestanding -nostdinc -nostdlib -nostartfiles -c uart.c -o uart.o
aarch64-none-elf-ld -nostdlib -nostartfiles start.o delays.o main.o mbox.o sd.o uart.o -T link.ld -o kernel8.elf
aarch64-none-elf-objcopy -O binary kernel8.elf kernel8.img

$ make run
qemu-system-aarch64 -M raspi3 -kernel kernel8.img -drive file=test.dd,if=sd,format=raw -serial stdio
EMMC: GPIO set up
EMMC: reset OK
sd_clk divisor 00000068, shift 00000006
EMMC: Sending command 00000000 arg 00000000
EMMC: Sending command 08020000 arg 000001AA
EMMC: Sending command 37000000 arg 00000000
EMMC: Sending command 29020000 arg 51FF8000
EMMC: CMD_SEND_OP_COND returned COMPLETE VOLTAGE FFFFFFFF80F9C000
EMMC: Sending command 02010000 arg 00000000
EMMC: Sending command 03020000 arg 00000000
EMMC: CMD_SEND_REL_ADDR returned 0000000045670000
sd_clk divisor 00000002, shift 00000000
EMMC: Sending command 07030000 arg 45670000
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 33220010 arg 00000000
EMMC: Sending command 37020000 arg 45670000
EMMC: Sending command 06020000 arg 45670002
EMMC: supports
sd_readblock lba 00000001 num 00000001
EMMC: Sending command 11220010 arg 00000200
sd_writeblock lba 00000001 num 00000001
EMMC: Sending command 18220000 arg 00000200
Boot counter AA550001 written to SD card.

$ make run
Boot counter AA550002 written to SD card.

$ make run
Boot counter AA550003 written to SD card.

$ make run
Boot counter AA550004 written to SD card.
```

## メモ

手元の環境では、第2セクタはMBRのコピーで、問題なし。
