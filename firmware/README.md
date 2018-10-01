# UF2 FPGA Loader

This repository contains a FPGA loader, derived from the uf2-samdx1 project.
This supports loading SPI flash devices commonly used to configure FPGAs 
through USB MSC (mass storage) using the UF2 file format.  

## UF2

**UF2 (USB Flashing Format)** is a name of a file format, developed by Microsoft, that is particularly
suitable for flashing devices over MSC devices. The file consists
of 512 byte blocks, each of which is self-contained and independent
of others.

Each 512 byte block consist of (see `uf2format.h` for details):
* magic numbers at the beginning and at the end
* address where the data should be flashed
* size of data
* data (up to 476 bytes; for SAMD it's 256 bytes so it's easy to flash in one go)

Thus, it's really easy for the microcontroller to recognize a block of
a UF2 file is written and immediately write it to flash.

* **UF2 specification repo:** https://github.com/Microsoft/uf2
* [#DeskOfLadyada UF24U ! LIVE @adafruit #adafruit #programming](https://youtu.be/WxCuB6jxLs0)

## Features

* USB MSC interface for writing UF2 files
* reading of the contests of the flash as an UF2 file via USB MSC
* FPGA access to USB CDC through a UART
* FPGA access to mcu peripherals through I2C

## Board identification

Configuration files for board `foo` are in `boards/foo/board_config.h` and `board.mk`. You can
build it with `make BOARD=foo`. You can also create `Makefile.user` file with `BOARD=foo`
to change the default.

The board configuration specifies the USB vendor/product name and ID,
as well as the volume label (main thing that the operating systems show).

There is also `BOARD_ID`, which is meant to be machine-readable and specific
to a given version of board hardware. The programming environment might use
this to suggest packages to be imported (i.e., a package for a particular
external flash chip, SD card etc.).

These configuration values can be read from `INFO_UF2.TXT` file.
Presence of this file can be tested to see if the board supports `UF2` flashing,
while contest, particularly `Board-ID` field, can be used for feature detection.

The current flash contents of the board is exposed as `CURRENT.UF2` file.
This file includes the bootloader address space. The last word of bootloader
space points to the string holding the `INFO_UF2.TXT` file, so it can be parsed
by a programming environment to determine which board does the `.UF2` file comes from.

## Build

### Requirements

* `make` and an Unix environment
* `node`.js in path (optional)
* `arm-none-eabi-gcc` in the path (the one coming with Yotta will do just fine)

### Build commands

The default board is `fpga_helper`. You can build a different one using:

```
make BOARD=metro_m0
```

If you're working on different board, it's best to create `Makefile.local`
with say `BOARD=metro` to change the default.
The names `fpga_helper` and `metro` refer to subdirectories of `boards/`.

There are various targets:
* `all` - just compile the board
* `burn` or `b` - compile and deploy to the board using openocd
* `logs` or `l` - shows logs
* `run` or `r` - burn, wait, and show logs

Typically, you will do:

```
make r
```

### Configuration

There is a number of configuration parameters at the top of `uf2.h` file.
Adjust them to your liking.

CDC and MSC together will work on Linux and Mac with no drivers.
On Windows, if you have drivers installed for the USB ID chosen,
then CDC might work and MSC will not work;
otherwise, if you have no drivers, MSC will work, and CDC will work on Windows 10 only.
Thus, it's best to set the USB ID to one for which there are no drivers.

The bootloader sits at 0x00000000, and the application starts at 0x00002000.

## License

See THIRD-PARTY-NOTICES.txt for the original SAM-BA bootloader license from Atmel.

The new code is licensed under MIT.
