#!/usr/bin/python

import serial, sys, glob, binascii

if len(sys.argv) >= 2:
    ser = serial.Serial(sys.argv[1])
else:
    for port in glob.glob('/dev/ttyACM*'):
        try:
            ser = serial.Serial(port, 9600)
        except Exception:
            pass # Fail
        else:
            break
    else:
        sys.exit("No ports found")



txt = raw_input("> ")

crc32 = binascii.crc32(txt) & 0xffffffff

for x in range(0, 10):
    ser.write(' ');

#send 3 Ctrl-C
for x in range(0, 3):
    ser.write('\x03');

#send length byte
txtlen = len(txt) & 0xFF;
ser.write(txtlen);

ser.write(crc32 >> 24 & 0xFF);
ser.write(crc32 >> 16 & 0xFF);
ser.write(crc32 >> 8 & 0xFF);
ser.write(crc32 & 0xFF);


ser.write(crc32 >> 24 & 0xFF ^ txtlen);
ser.write(crc32 >> 16 & 0xFF ^ txtlen);
ser.write(crc32 >> 8 & 0xFF ^ txtlen);
ser.write(crc32 & 0xFF ^ txtlen );

for c in txt:
  ser.write(c);

