#!/bin/sh

avr-gcc -std=c++17 -g -Os -mmcu="atmega328p" -Xlinker -Map=arduino.map src/uart.cpp src/msghandler.cpp src/main.cpp -o arduino.elf
avr-objcopy -j .text -j .data -O ihex arduino.elf arduino.hex
avr-size --format=avr --mcu="atmega328p" arduino.elf
