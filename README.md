# Bad Ducky

## Summary 
It is based on CJMCU BadUsb (ATMEGA32u4 - Arduino Leonardo clone) board with onboard card reader, which you can buy on [aliexpress](
https://www.aliexpress.com/item/CJMCU-Virtual-Keyboard-Badusb-USB-TF-Memory-Keyboard-ATMEGA32U4/32815828963.html?spm=a2g0s.9042311.0.0.mhzoBn). My goal was to create something compatible with Rubber Ducky scripts, while having ability to easily choose which script to execute without modifying the hardware (without adding DIP switches).

![BadUsb](https://i.gyazo.com/49dda63e0360c9cc874be7322f2f2960.jpg)

## What is so special about this one?
- it is completely compatible with [Rubber Ducky scripts](https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Duckyscript) (all commands are supported)
- scripts does not require compiling/encoding
- there is no limit on payload size (the only limit is SD card size)
- it has three modes (continuous delivery, one-time delivery and management mode for uploading a new sketch without executing the payload)

For more info on how to get started check [wiki pages](https://github.com/insight1620/CJMCU-BadUSB/wiki)
