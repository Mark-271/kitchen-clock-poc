#ifndef DELAY_H
#define DELAY_H

#include <libopencm3/cm3/assert.h>

/** CPU cycles per 1 iteration of loop in ldelay() */
#define CYCLES_PER_LOOP        3UL
/** How many CPU cycles to wait for 1 usec */
#define CYCLES_PER_USEC        48UL    /* for 48 MHz CPU frequency */
/** Delay for "d" micro-seconds */
#define udelay(d)        ldelay((d) * CYCLES_PER_USEC)
/** Delay for "d" milliseconds */
#define mdelay(d)        udelay((d) * 1000UL)

/**
 * Delay for "cycles" number of machine cycles.
 *
 * @param cycles Number of CPU cycles to delay (must be >= CYCLES_PER_LOOP)
 *
 * @note Interrupts should be disabled during this operation
 */
static inline __attribute__((always_inline)) void ldelay(unsigned long cycles)
{
    unsigned long loops = cycles / CYCLES_PER_LOOP;

    cm3_assert(cycles >= CYCLES_PER_LOOP);

    __asm__ __volatile__ (
        "1:\n" "subs %0, %1, #1\n"
        "bne 1b"
        : "=r" (loops)
        : "0" (loops));
}

#endif /* DELAY_H */
