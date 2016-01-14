# avr-osccal

A C implementation of the OSCCAL algorithm in Atmel's App Note AVR053

## Description

Atmel's AVR MCUs have internal oscillators that come factory-calibrated with a
10% tolerance.  Applications using the internal oscillator and requiring an
accurate clock should calculate a new calibration value given in-circuit
voltage and temperature conditions.

App Note [AVR053](http://www.atmel.com/images/doc2555.pdf) describes a protocol
for doing this automatically with the ISP interface.  This is an implementation
in C, for convenience and portability.  It may be used in conjunction with
either `avrdude` or `atprogram` for the actual calibration.

For maximum portability, this implementation follows the recommendations of
AVR053 in avoiding interrupts and using the 8-bit timer, which should allow it
to run on any AVR device.

## License

This library is under the BSD-2-Clause license.

## Usage

Edit the constants at the top of the makefile.  Then run:
```
make
make install
make calibrate
```

The calculated OSCCAL value will be stored in EEPROM at address 0x00.  Note
that the `EESAVE` fuse must be set, or it will be erased as soon as you flash
your final code.

Your final code should then read EEPROM address 0x00 during startup and write
it to OSCCAL.
