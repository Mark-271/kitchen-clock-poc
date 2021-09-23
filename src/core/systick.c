#include <core/systick.h>
#include <tools/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>

#define SYSTICK_FREQ		1000 /* overflows per second */
#define AHB_FREQUENCY		24000000UL
#define SYSTICK_RELOAD_VAL	(AHB_FREQUENCY / SYSTICK_FREQ)
#define AHB_TICKS_PER_USEC	(AHB_FREQUENCY / 1000000)
#define USEC_PER_MSEC		1000
#define NSEC_PER_USEC		1000
#define NSEC_PER_MSEC		1000000
#define MSEC_PER_SEC		1000

static uint32_t ticks;

/**
 * Systick handler.
 *
 * Since systick handler is an entry of vector table, it is also copied
 * to RAM during vector table relocation. However there is no possibility
 * to request systick by its irq number when interacting with irq manager
 * for the reason that systick is implemented inside Cortex-M3 core, and is
 * considered primarely as exception, not an interrupt. As systick handler
 * is defined as a weak symbol, caller should redefine it.
 */
void sys_tick_handler(void)
{
	WRITE_ONCE(ticks, ticks + 1);
}

void systick_get_time(struct systick_time *t)
{
	uint32_t ms;
	uint32_t ns;
	unsigned long flags;

	t->sec = 0;
	t->nsec = 0;

	enter_critical(flags);
	ms = ticks;
	t->nsec = (SYSTICK_RELOAD_VAL - systick_get_value()) /
		   AHB_TICKS_PER_USEC * NSEC_PER_USEC;
	exit_critical(flags);

	while (ms > MSEC_PER_SEC) {
		ms -= MSEC_PER_SEC;
		t->sec++;
	}

	ns = ms * NSEC_PER_MSEC;
	t->nsec += ns;

	while (t->nsec > NSEC_PER_SEC) {
		t->nsec -= NSEC_PER_SEC;
		t->sec++;
	}
}

uint64_t systick_calc_diff(const struct systick_time *t1,
			   const struct systick_time *t2)
{
	uint64_t ts1;
	uint64_t ts2;

	ts1 = t1->sec * NSEC_PER_SEC + t1->nsec;
	ts2 = t2->sec * NSEC_PER_SEC + t2->nsec;

	if (ts1 > ts2)
		ts2 += UINT64_MAX;

	return ts2 - ts1;
}

/*
 * Precision: +/-1 msec.
 */
uint32_t systick_get_time_ms(void)
{
	return ticks;
}

uint32_t systick_get_time_us(void)
{
	uint32_t us;

	us = (SYSTICK_RELOAD_VAL - systick_get_value()) / AHB_TICKS_PER_USEC;
	us += READ_ONCE(ticks) * USEC_PER_MSEC;

	return us;
}

uint32_t systick_get_time_ns(void)
{
	uint32_t ns;

	ns = (SYSTICK_RELOAD_VAL - systick_get_value()) / AHB_TICKS_PER_USEC
	     * NSEC_PER_USEC;
	ns += READ_ONCE(ticks) * NSEC_PER_MSEC;

	return ns;
}

/* Calculate timestamp difference in nanoseconds */
uint32_t systick_calc_diff_ns(uint32_t t1, uint32_t t2)
{
	if (t1 > t2)
		t2 += UINT32_MAX;

	return t2 - t1;
}

/* Calculate timestamp difference in milliseconds */
uint32_t systick_calc_diff_ms(uint32_t t1, uint32_t t2)
{
	if (t1 > t2)
		t2 += UINT32_MAX;

	return t2 - t1;
}

/**
 * Initialize systick timer.
 *
 * @return 0 or -1 on error
 */
int systick_init(void)
{
	if (!systick_set_frequency(SYSTICK_FREQ, AHB_FREQUENCY))
		return -1;

	systick_clear();
	systick_interrupt_enable();
	systick_counter_enable();

	return 0;
}

/**
 * De-initialize systick timer.
 */
void systick_exit(void)
{
	systick_counter_disable();
	systick_interrupt_disable();
}
