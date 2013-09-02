GrrCON_2013_Badge
=================

Schematics, Board Designs and code for GrrCON 2013 Badges.

Schematic and Board layouts require Eagle 6.4.

Badge is based on Arduino Uno Rev 3.  http://arduino.cc

The board layout and schematic are released under the Creative Commons Share Alike license.  The GrrCON "skull" outline of the board should not be used without permission.

When programming the bootloader on the Main ATMEGA328P processor fuses are:
efuse:0x06 hfuse:0xD6 lfuse:0x7F

Custom bootloader and board definition in the hardware directory.

https://github.com/arduino/Arduino/blob/master/hardware/arduino/firmwares/atmegaxxu2/Arduino-COMBINED-dfu-usbserial-atmega16u2-Uno-Rev3.hex
lfuse:w:0xFF:m hfuse:w:0xD9:m efuse:w:0xF4:m lock:w:0x0F:m
