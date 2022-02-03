#!/bin/sh
avrdude -v -c arduino -patmega328p -P/dev/ttyUSB0 -b115200 -U flash:w:arduino.hex
