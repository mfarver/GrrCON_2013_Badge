GrrCON_2013_Badge
=================

Schematics, Board Designs and code for GrrCON 2013 Badges.

Schematic and Board layouts require Eagle 6.4.

Badge is based on Ardunio Uno Rev 3.  http://ardunio.cc

The board layout and schematic are released under the Creative Commons Share Alike license.  The "skull" outline of the board is

When programming the bootloader on the Main ATMEGA328P processor fuses are:
efuse:0x05 hfuse:w:0xD6 lfuse:w:0xFF:m
Firmware ships with Ardunio Environment in hardware/ardunio/bootloads/optiboot directory

https://github.com/arduino/Arduino/blob/master/hardware/arduino/firmwares/atmegaxxu2/Arduino-COMBINED-dfu-usbserial-atmega16u2-Uno-Rev3.hex
lfuse:w:0xFF:m hfuse:w:0xD9:m efuse:w:0xF4:m lock:w:0x0F:m
