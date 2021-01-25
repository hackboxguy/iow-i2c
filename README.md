# iow-i2c
iow-i2c utility is used for testing i2c slave devices using usb-to-i2c-dongle(on Linux platform) from Code-Mercenaries(IO-Warrior28)

As of now, this utility simply sends two byte count value[0x11 to 0xFF] to slave device 0xA0.

e.g: 
 1. open io-warrio-device, i.e: /dev/usb/iowarrior1
 2. i2c-write ==> 0xA0 0x11 0x11
 3. i2c-read  <== 0xA0 (reaback previously written two bytes)
 4. verify if written and read-back bytes match.
 5. increase count i.e 0x11 0x11 to 0x22 0x22
 6. repeat 15 times (i.e exit when count reaches 0xFF)

