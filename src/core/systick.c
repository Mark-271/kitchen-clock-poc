// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <core/systick.h>
#include <tools/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>

#define SYSTICK_FREQ		1000UL /* overflows per second */
#define AHB_FREQUENCY		24000000UL
#define SYSTICK_RELOAD_VAL	((AHB_FREQUENCY / SYSTICK_FREQ) - 1)
#define NSEC_PER_MSEC		1000000UL
#define MSEC_PER_SEC		1000UL
/* HR Timer resolution: 1/24000000 = 41.666 nsec/tick = (125/3) nsec/tick */
#define HR_RES_NUM		125
#define HR_RES_DENOM		3
#define DIV_ROUND_CLOSEST	(HR_RES_NUM / HR_RES_DENOM)
#define systick_get_reg()	STK_CVR

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

/**
 * Get timestamp.
 *
 * Data precision is 125 ns.
 * When needed a few timestamps, then every call of function must be provided
 * with its own systick_time instance.
 *
 * @param[out] t Pointer to systick_time object storing time data.
 */
void systick_get_time(struct systick_time *t)
{
	uint32_t ms;
	uint32_t ns = 0;
	uint32_t reg;
	unsigned long flags;

	enter_critical(flags);
	ms = ticks;
	reg = systick_get_reg();
	exit_critical(flags);

	/*
	 * Calculate number of nanoseconds stored in STK_CVR register with
	 * granularity of 3 counter ticks due to fractionary number of
	 * nanoseconds per tick
	 */
	ns = (SYSTICK_RELOAD_VAL - reg) * DIV_ROUND_CLOSEST;
	ns += (ms % MSEC_PER_SEC) * NSEC_PER_MSEC;

	t->sec = ms / MSEC_PER_SEC;
	t->nsec = ns;
}

/**
 * Get difference between two time stamps.
 *
 * @param[in] t1 Pointer to systick_time object containig first timestamp
 * @param[in] t2 Pointer to systick_time object that carries second timestamp
 * @return time in nanoseconds elapsed between two events specified by
 * 	   input parameters
 */
uint64_t systick_calc_diff(const struct systick_time *t1,
			   const struct systick_time *t2)
{
	uint64_t t1_ns;
	uint64_t t2_ns;
	uint64_t t2_sec;

	/* Timestamp t1 must precede timestamp t2 */
	if ((t1->sec > t2->sec) ||
	   ((t1->sec == t2->sec) && (t1->nsec > t2->nsec)))
		t2_sec = (uint64_t)t2->sec + UINT32_MAX + 1;
	else
		t2_sec = t2->sec;

	t1_ns = (uint64_t)t1->sec * NSEC_PER_SEC + t1->nsec;
	t2_ns = t2_sec * NSEC_PER_SEC + t2->nsec;

	return t2_ns - t1_ns;
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
