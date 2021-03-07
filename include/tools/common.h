#ifndef TOOLS_COMMON_H
#define TOOLS_COMMON_H

#include <libopencm3/cm3/assert.h>

#define BIT(n)			(1 << (n))
#define ARRAY_SIZE(a)		(sizeof(a) / sizeof(a[0]))
#define UNUSED(x)		(void)x

/**
 * Enter critical section (store IRQ flags and disable interrupts).
 *
 * This function serves a purpose of guarding some code from being interrupted
 * by ISR. For example it can be used to safely read/write variable which is
 * also being changed in ISR. Note that it's not necessary to mark such
 * variables with "volatile" modifier when already guarding it with critical
 * section (refer to @ref exit_critical() documentation for details). Keep the
 * critical section as small and fast as possible!
 *
 * The @p flags parameter indicates whether interrupts are enabled or disabled
 * right now (before disabling interrupts). It's often desired to restore such
 * interrupts enabled/disabled state instead of just enabling interrupts. This
 * function stores IRQ flags into provided variable instead of using global
 * variable, to avoid re-entrance issues.
 *
 * Memory barriers are not needed when disabling interrupts (see AN321).
 *
 * @param[out] flags Will contain IRQ flags value before disabling interrupts;
 *                   must have "unsigned long" type
 */
#define enter_critical(flags)						\
do {									\
	__asm__ __volatile__ (						\
		"mrs %0, primask\n"	/* save PRIMASK to "flags" */	\
		"cpsid i"		/* disable interrupts */	\
		: "=r" (flags)						\
		:							\
		: "memory");						\
} while (0)

/**
 * Exit critical section (restore saved IRQ flags).
 *
 * Restores interrupts state (enabled/disabled) stored in @ref enter_critical().
 *
 * Contains memory barriers:
 *   - compiler barrier ("memory"): to prevent caching the values in registers;
 *     this way we don't have to use "volatile" modifier for variables that
 *     are being changed in ISRs
 *   - processor barrier (ISB): so that pending interrupts are run right after
 *     ISB instruction (as recommended by ARM AN321 guide)
 *
 * @param[in] flags Previously saved IRQ flags.
 */
#define exit_critical(flags)						\
do {									\
	__asm__ __volatile__ (						\
		"msr primask, %0\n" /* load PRIMASK from "flags" */	\
		"isb"							\
		:							\
		: "r" (flags)						\
		: "memory");						\
} while (0)

/**
 * Wait for some event (condition) to happen, breaking off on timeout.
 *
 * This is blocking wait, of course, as we don't use context switching.
 * Be aware of watchdog interval!
 *
 * @param cond C expression (condition) for the event to wait for
 * @param timeout Timeout in msec
 * @return 0 if condition is met or -1 on timeout
 */
#define wait_event_timeout(cond, timeout)				\
({									\
	uint32_t _t1;							\
	int _ret = 0;							\
									\
	_t1 = systick_get_time_ms();					\
									\
	while (!(cond)) {						\
		uint32_t _t2 = systick_get_time_ms();			\
		uint32_t _elapsed = systick_calc_diff_ms(_t1, _t2);	\
									\
		if (_elapsed > (timeout)) {				\
			_ret = -1;					\
			break;						\
		}							\
	}								\
									\
	_ret;								\
})

/** CPU cycles per 1 iteration of loop in ldelay() */
#define CYCLES_PER_LOOP		3UL
/** How many CPU cycles to wait for 1 usec */
#define CYCLES_PER_USEC		24UL	   /* for 24 MHz CPU frequency */
/** Delay for "d" micro-seconds */
#define udelay(d)		ldelay((d) * CYCLES_PER_USEC)
/** Delay for "d" milliseconds */
#define mdelay(d)		udelay((d) * 1000UL)

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

void __attribute__((__noreturn__)) hang(void);

#endif /* TOOLS_COMMON_H */
