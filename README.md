# USB Morse code keyboard

This repository contains code for a Raspberry Pi Pico (or any other RP2040-based board) that makes it a USB Morse code keyboard.

Wire your Morse key (or any kind of switch or button) to GPIO14 and GND.

The transmission speed is currently hardcoded.

You can compile it with the following commands (or use the precompiled UF2 file):

```
git clone https://github.com/jfedor2/pico-morse.git
cd pico-morse
git submodule update --init
mkdir build
cd build
cmake ..
make
```
