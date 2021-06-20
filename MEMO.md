# SDカードイメージの作成

```
$ mkdir sdcard && cd sdcard
$ wget -O bootcode.bin https://github.com/raspberrypi/firmware/raw/master/boot/bootcode.bin
$ wget -O fixup.dat https://github.com/raspberrypi/firmware/raw/master/boot/fixup.dat
$ wget -O start.elf https://github.com/raspberrypi/firmware/raw/master/boot/start.elf
$ qemu-img create -f raw raspi3.img 128M
Formatting 'raspi3.img', fmt=raw size=134217728
$ mkfs.fat -n 'RASPI3' -s 2 -f 2 -R 32 -F 32 --mbr=y raspi3.img
mkfs.fat 4.2 (2021-01-31)
dspace@mini:~/raspi_os/raspi3-tutorial/sdcard$ mkdir mnt
dspace@mini:~/raspi_os/raspi3-tutorial/sdcard$ hdiutil attach -mountpoint mnt raspi3.img
/dev/disk2          	                               	/Users/dspace/raspi_os/raspi3-tutorial/sdcard/mnt
dspace@mini:~/raspi_os/raspi3-tutorial/sdcard$ cp bootcode.bin fixup.dat start.elf mnt
dspace@mini:~/raspi_os/raspi3-tutorial/sdcard$ ls mnt
bootcode.bin  fixup.dat  start.elf
dspace@mini:~/raspi_os/raspi3-tutorial/sdcard$ hdiutil detach mnt
"disk2" ejected.
```

# QEMUのデバイスツリーを表示

- 参考記事: [Raspberry Pi 3のSDHOSTを動かす](https://qiita.com/eggman/items/decd5cef42c9c25a383a)

```
$ qemu-system-aarch64 -M raspi3 -nographic -S
QEMU 5.2.0 monitor - type 'help' for more information
(qemu) info qtree
bus: main-system-bus
  type System
  dev: bcm2836-control, id ""
    gpio-out "fiq" 4
    gpio-out "irq" 4
    gpio-in "gpu-fiq" 1
    gpio-in "gpu-irq" 1
    gpio-in "cntvirq" 4
    gpio-in "cnthpirq" 4
    gpio-in "cntpnsirq" 4
    gpio-in "cntpsirq" 4
    mmio 0000000040000000/0000000000000100
  dev: unimplemented-device, id ""
    size = 256 (0x100)
    name = "bcm2835-sdramc"
    mmio ffffffffffffffff/0000000000000100
  dev: unimplemented-device, id ""
    size = 4096 (0x1000)
    name = "bcm2835-v3d"
    mmio ffffffffffffffff/0000000000001000
  dev: unimplemented-device, id ""
    size = 32768 (0x8000)
    name = "bcm2835-ave0"
    mmio ffffffffffffffff/0000000000008000
  dev: unimplemented-device, id ""
    size = 32768 (0x8000)
    name = "bcm2835-dbus"
    mmio ffffffffffffffff/0000000000008000
  dev: unimplemented-device, id ""
    size = 128 (0x80)
    name = "bcm2835-otp"
    mmio ffffffffffffffff/0000000000000080
  dev: unimplemented-device, id ""
    size = 32 (0x20)
    name = "bcm2835-i2c2"
    mmio ffffffffffffffff/0000000000000020
  dev: unimplemented-device, id ""
    size = 32 (0x20)
    name = "bcm2835-i2c1"
    mmio ffffffffffffffff/0000000000000020
  dev: unimplemented-device, id ""
    size = 32 (0x20)
    name = "bcm2835-i2c0"
    mmio ffffffffffffffff/0000000000000020
  dev: unimplemented-device, id ""
    size = 256 (0x100)
    name = "bcm2835-spis"
    mmio ffffffffffffffff/0000000000000100
  dev: unimplemented-device, id ""
    size = 32 (0x20)
    name = "bcm2835-spi0"
    mmio ffffffffffffffff/0000000000000020
  dev: unimplemented-device, id ""
    size = 256 (0x100)
    name = "bcm2835-smi"
    mmio ffffffffffffffff/0000000000000100
  dev: unimplemented-device, id ""
    size = 256 (0x100)
    name = "bcm2835-i2s"
    mmio ffffffffffffffff/0000000000000100
  dev: unimplemented-device, id ""
    size = 276 (0x114)
    name = "bcm2835-powermgt"
    mmio ffffffffffffffff/0000000000000114
  dev: unimplemented-device, id ""
    size = 64 (0x40)
    name = "bcm2835-sp804"
    mmio ffffffffffffffff/0000000000000040
  dev: unimplemented-device, id ""
    size = 4096 (0x1000)
    name = "bcm2835-txp"
    mmio ffffffffffffffff/0000000000001000
  dev: dwc2-usb, id ""
    gpio-out "sysbus-irq" 1
    usb_version = 2 (0x2)
    mmio ffffffffffffffff/0000000000011000
    bus: usb-bus.0
      type usb-bus
  dev: bcm2835-mphi, id ""
    gpio-out "sysbus-irq" 1
    mmio ffffffffffffffff/0000000000001000
  dev: bcm2835_gpio, id ""
    gpio-out "" 54
    mmio ffffffffffffffff/0000000000001000
    bus: sd-bus
      type sd-bus
  dev: bcm2835-thermal, id ""
    mmio ffffffffffffffff/0000000000000008
  dev: bcm2835-dma, id ""
    gpio-out "sysbus-irq" 16
    mmio ffffffffffffffff/0000000000001000
    mmio ffffffffffffffff/0000000000000100
  dev: bcm2835-sdhost, id ""
    gpio-out "sysbus-irq" 1
    mmio ffffffffffffffff/0000000000001000
    bus: sd-bus
      type bcm2835-sdhost-bus
  dev: generic-sdhci, id ""
    gpio-out "sysbus-irq" 1
    sd-spec-version = 3 (0x3)
    uhs = 0 (0x0)
    vendor = 0 (0x0)
    capareg = 86062260 (0x52134b4)
    maxcurr = 0 (0x0)
    pending-insert-quirk = true
    mmio ffffffffffffffff/0000000000000100
    bus: sd-bus
      type sdhci-bus
      dev: sd-card, id ""
        spec_version = 2 (0x2)
        drive = "sd0"
        spi = false
  dev: bcm2835-rng, id ""
    mmio ffffffffffffffff/0000000000000010
  dev: bcm2835-property, id ""
    gpio-out "sysbus-irq" 1
    board-rev = 10494082 (0xa02082)
    mmio ffffffffffffffff/0000000000000010
  dev: bcm2835-fb, id ""
    gpio-out "sysbus-irq" 1
    vcram-base = 1006632960 (0x3c000000)
    vcram-size = 67108864 (0x4000000)
    xres = 640 (0x280)
    yres = 480 (0x1e0)
    bpp = 16 (0x10)
    pixo = 1 (0x1)
    alpha = 2 (0x2)
    mmio ffffffffffffffff/0000000000000010
  dev: bcm2835-mbox, id ""
    gpio-in "" 9
    gpio-out "sysbus-irq" 1
    mmio ffffffffffffffff/0000000000000400
  dev: bcm2835-aux, id ""
    gpio-out "sysbus-irq" 1
    chardev = ""
    mmio ffffffffffffffff/0000000000000100
  dev: pl011, id ""
    gpio-out "sysbus-irq" 6
    clock-in "clk" freq_hz=4.799963e+07
    chardev = "serial0"
    mmio ffffffffffffffff/0000000000001000
  dev: bcm2835-sys-timer, id ""
    gpio-out "sysbus-irq" 4
    mmio ffffffffffffffff/0000000000000020
  dev: bcm2835-cprman, id ""
    clock-out (alias) "emmc2-out" freq_hz=0.000000e+00
    clock-out (alias) "emmc-out" freq_hz=2.000000e+08
    clock-out (alias) "aveo-out" freq_hz=0.000000e+00
    clock-out (alias) "arm-out" freq_hz=0.000000e+00
    clock-out (alias) "sdram-out" freq_hz=0.000000e+00
    clock-out (alias) "pulse-out" freq_hz=0.000000e+00
    clock-out (alias) "vec-out" freq_hz=0.000000e+00
    clock-out (alias) "uart-out" freq_hz=4.799963e+07
    clock-out (alias) "timer-out" freq_hz=1.000002e+06
    clock-out (alias) "tsens-out" freq_hz=1.920000e+06
    clock-out (alias) "td1-out" freq_hz=0.000000e+00
    clock-out (alias) "td0-out" freq_hz=0.000000e+00
    clock-out (alias) "tec-out" freq_hz=0.000000e+00
    clock-out (alias) "smi-out" freq_hz=0.000000e+00
    clock-out (alias) "slim-out" freq_hz=0.000000e+00
    clock-out (alias) "pwm-out" freq_hz=0.000000e+00
    clock-out (alias) "pcm-out" freq_hz=0.000000e+00
    clock-out (alias) "otp-out" freq_hz=4.800000e+06
    clock-out (alias) "hsm-out" freq_hz=0.000000e+00
    clock-out (alias) "gp2-out" freq_hz=3.276800e+04
    clock-out (alias) "gp1-out" freq_hz=2.500000e+07
    clock-out (alias) "gp0-out" freq_hz=0.000000e+00
    clock-out (alias) "dpi-out" freq_hz=0.000000e+00
    clock-out (alias) "dsi0p-out" freq_hz=0.000000e+00
    clock-out (alias) "dsi0e-out" freq_hz=0.000000e+00
    clock-out (alias) "ccp2-out" freq_hz=0.000000e+00
    clock-out (alias) "cam1-out" freq_hz=0.000000e+00
    clock-out (alias) "cam0-out" freq_hz=0.000000e+00
    clock-out (alias) "v3d-out" freq_hz=0.000000e+00
    clock-out (alias) "isp-out" freq_hz=0.000000e+00
    clock-out (alias) "h264-out" freq_hz=0.000000e+00
    clock-out (alias) "perii-out" freq_hz=0.000000e+00
    clock-out (alias) "peria-out" freq_hz=0.000000e+00
    clock-out (alias) "sys-out" freq_hz=0.000000e+00
    clock-out (alias) "vpu-out" freq_hz=0.000000e+00
    clock-out (alias) "gnric-out" freq_hz=0.000000e+00
    xosc-freq-hz = 19200000 (0x124f800)
    mmio ffffffffffffffff/0000000000002000
  dev: bcm2835-ic, id ""
    gpio-in "arm-irq" 8
    gpio-in "gpu-irq" 64
    mmio ffffffffffffffff/0000000000000200
  dev: bcm2835-peripherals, id ""
    gpio-out "sysbus-irq" 2
    mmio 000000003f000000/0000000001000000
```
