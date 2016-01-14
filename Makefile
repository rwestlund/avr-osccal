# Copyright (c) 2016 Randy Westlund, All rights reserved.
# This code is under the BSD-2-Clause license

# Used to build the OSCCAL calibration code, written in BSD make

# edit these with your values
MCU_TARGET = atmega328
PROGRAMMER = avrisp2
# frequency of the calibration clock provided by the programmer
ISP_FREQ = 24592
# MCU clock frequency
CLOCK_FREQ = 1000000

DEFINES = -DISP_FREQ=${ISP_FREQ} -DCLOCK_FREQ=${CLOCK_FREQ}
CC = avr-gcc
OBJCOPY = avr-objcopy
UPLOADER = avrdude
OBJDUMP = avr-objdump
SIZE = avr-size

CFLAGS = -Wall -Werror -std=c99 -pedantic -Os -mmcu=${MCU_TARGET} ${DEFINES}
LDFLAGS = -Wl,-Map
OBJCOPYFLAGS = -j .text -j .data -O ihex
OBJDUMPFLAGS = -h -S

PROG = osccal
SRCS = osccal.c

all: ${PROG}.{hex,lst}

${PROG}.hex: ${PROG}.elf
	${OBJCOPY} ${OBJCOPYFLAGS} ${.ALLSRC} ${.TARGET}
	${SIZE} ${.TARGET}

${PROG}.elf: ${PROG}.c
	${CC} ${CFLAGS} -o ${.TARGET} ${SRCS}
	${SIZE} ${.TARGET}

${PROG}.lst: ${PROG}.elf
	${OBJDUMP} ${OBJDUMPFLAGS} ${.ALLSRC} > ${.TARGET}

install:
	${UPLOADER} -p ${MCU_TARGET} -c ${PROGRAMMER} -P usb -e -U flash:w:${PROG}.hex

calibrate:
	${UPLOADER} -p ${MCU_TARGET} -c ${PROGRAMMER} -P usb -O

clean:
	rm -f *.hex *.o *.lst *.map *.elf
