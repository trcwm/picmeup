# Wiggler UART protocol

## General information

* Communications currently run at 115.2kbaud.
* All pins are defined as input pins when the wiggler starts up.
* Reading a pin makes it an input.
* Writing to a pin makes it an output.

## Messages (PC to Wiggler)

All messages are in ASCII. CR and LF are ignored.

* Wxy : write/set pin x (ASCII hex) to state y=0 or 1.
* wxyz : write serial bitstream to pin x (ASCII hex) with 8-bit data yz (ASCII hex).
* Rx  : read pin x (ASCII hex) and shift into LSB of 8bit result register.
* rx  : read pin x (ASCII hex) and shift into USB of 8bit result register.
* Pxyz: set wiggler parameter x (ASCII hex) to yz (ASCII hex)

* Cxy : check status of pin x (y = 0 or 1), send error if no match.
* CL  : clear 8bit result register.
* T   : transmit result register to UART in ASCII hex.

* Mxy : match result register with xy (ASCII hex), send error if no match.

* Dxy : defined canned cycle program with code xy (ASCII hex).
* E   : exit canned cycle program.
* Xxy : execute canned cycle program with code xy (ASCII hex), return 'K' when done.

* Sxy : send ASCII xy back to the PC.
* I   : report version.

* WTxy : wait xy (in ASCII hex). In units of 1 millisecond.

## Messages (Wiggler to PC)

* Exy : error code, where xy are the argument of the PC C or M command.
* Rxy : result code from PC T command. xy are in ASCII hex form.
* K   : returned by canned program on success.
* Is  : returned by PC I command. Human readable version string and pinout information, null terminated.

## Parameters
* P0  : delay between each pin change in 10 usec steps, 0 = minimal delay.
