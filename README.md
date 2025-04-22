# ESP32 Pulse Counter

## Introduction

This is a simple ESP32 application for counting pulses on an GPIO pin.  It is
intended for debugging problems with step pulse generation on CNC controllers,
but could be used for other purposes.  The current pulse count is displayed on the USB serial port.

## Pins

* GPIO 4 - This is the pulse input.  Connect the signal source (0-3.3V) to it.  When a rising edge occurs
on this input, a counter will be incremented by one.
* GPIO 2 - This is the direction input.  Connect the direction signal (0-3.3V) to it.  When it is high or not connected, the counter will increment; when low, the counter will decrement.
* GPIO 0 - Counter reset input.  A transition on this pin will reset the counter.  This pin is normally connected to the "BOOT" button on an ESP32, so it is not necessary to connect an external switch; you can just push the BOOT button.
* GPIO 18 - Test output.  This generates a 100 kHz signal.  You can connect it to GPIO 4 to test the program.

You also need to connect GND between the Pulse Counter ESP32 and the external device that is generating the pulses.

## Installation

Get the files bootloader.bin, partitions.bin, and firmware.bin from the *binaries* folder in this repo.

Use esptool to install it on an ESP32 with a command like:

```
python esptool.py --baud 460800 --before default_reset --after hard_reset write_flash -z --flash_size 4MB 0x1000 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
```

You might need to add extra arguments like `--port COM5` or `--chip esp32` or `--flash_mode dio` depending on your system and ESP32 device.  If you have installation problems, look for esptool help on the Internet.

## Usage

Use a serial monitor program to connect to the ESP32 at 115200 baud.  Any serial monitor should be okay, for example Arduino Serial Monitor, PuTTY, TeraTermPro, screen (MacOS and Linux), ...

When the pulse counter program starts, it will display some startup messages, then every second it will print a message like
```
counts: 56789
```
The number is the number of pulses that were seen on the input GPIO since the counter was last cleared.

To clear the counter, press the BOOT button on the ESP32 module (or an external pushbutton if you have connected one to GPIO 0).

## Compiling

This program is set up for compling via PlatformIO.  If you have PlatformIO installed, you can compile it with

```
pio run
```

in the top level directory or compile and upload with

```
pio run -t upload
```

If you are using an IDE that supports PlatformIO, such as VSCode, there is probably a button you can click instead of typing those commands.


