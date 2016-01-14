/* Copyright (c) 2016 Randy Westlund, All rights reserved.
   This code is under the BSD-2-Clause license */

/* force floating point math */
#define CAL_CLOCK_TICKS 40.0
#define TARGET_TICKS ((CAL_CLOCK_TICKS*CLOCK_FREQ)/ISP_FREQ)
#define DEVIATION (0.01*TARGET_TICKS)

#define CLOCK_CORRECT 0
#define CLOCK_TOO_FAST 1
#define CLOCK_TOO_SLOW 2

/* calibration clock */
#define MOSI_PIN PB3
/* signals back to the programmer */
#define MISO_PIN PB4

#include <avr/io.h> /* pin definitions */
#include <avr/eeprom.h> /* eeprom access functions */

static inline uint8_t try_value(uint8_t osccal);

int main(void) {
    /* enable pull-up resistor on MOSI */
    PORTB |= _BV(MOSI_PIN);
    /* set MISO as output */
    DDRB |= _BV(MISO_PIN);

    /* set MISO high to signal the programmer that we're ready */
    PORTB |= _BV(MISO_PIN);

    /* wait for calibration clock */
    loop_until_bit_is_clear(PINB, MOSI_PIN);
    /* start at half value for binary search */
    uint8_t osccal = 255/2;
    /* step would be half if not for integer rounding */
    uint8_t step = osccal;
    uint8_t status;

    for(;;) {
        status = try_value(osccal);

        if (status == CLOCK_CORRECT) break;
        if (status == CLOCK_TOO_FAST) osccal -= step;
        else if (status == CLOCK_TOO_SLOW) osccal += step;
        /* keep going only if we still have a step */
        if (!step) break;
        step = step/2;
    }

    /* fail */
    if (status != CLOCK_CORRECT) {
        PORTB &= _BV(MISO_PIN);
        for(;;) {}
    }
    /* store in eeprom */
    eeprom_update_byte(0x00, osccal);

    /* toggle MISO line 8 times to signal success */
    uint8_t i;
    for (i = 0; i < 8; i++) {
        /* wait for falling edge */
        loop_until_bit_is_clear(PINB, MOSI_PIN);
        /* toggle MISO */
        PINB |= _BV(MISO_PIN);
        /* wait for rising edge */
        loop_until_bit_is_set(PINB, MOSI_PIN);
    }

    /* we're done; enter infinite loop */
    for(;;) {}
}

/* try a calibration value and return the result */
static inline uint8_t try_value(uint8_t osccal) {
    OSCCAL = osccal;
    uint8_t high_count = 0;
    /* start counter 0 to count clock ticks */
    TCNT0 = 0;
    TCCR0B = _BV(CS00);

    for (int i = 0; i < CAL_CLOCK_TICKS; i++) {
        /* check for overflow */
        if (TIFR0 & _BV(TOV0)) { TIFR0 |= _BV(TOV0); high_count++; }
        /* wait for rising edge */
        loop_until_bit_is_set(PINB, MOSI_PIN);
        /* check for overflow */
        if (TIFR0 & _BV(TOV0)) { TIFR0 |= _BV(TOV0); high_count++; }
        /* wait for falling edge */
        loop_until_bit_is_clear(PINB, MOSI_PIN);
    }

    /* stop counter */
    TCCR0B &= ~_BV(CS00);
    /* check for overflow */
    if (TIFR0 & _BV(TOV0)) { TIFR0 |= _BV(TOV0); high_count++; }

    /* subtract a few ticks because it takes a few instructions to respond */
    uint16_t ticks = ((((uint16_t)high_count)<<8 ) | TCNT0) - 2;
    /* if too slow */
    if (ticks < (uint16_t)(TARGET_TICKS - DEVIATION)) return CLOCK_TOO_SLOW;
    /* if too fast */
    else if (ticks > (uint16_t)(TARGET_TICKS + DEVIATION)) return CLOCK_TOO_FAST;
    /* if within acceptable deviation */
    else return CLOCK_CORRECT;
}
    
