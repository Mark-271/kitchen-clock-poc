// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <board.h>
#include <core/irq.h>
#include <core/log.h>
#include <core/reset.h>
#include <core/sched.h>
#include <core/swtimer.h>
#include <core/systick.h>
#include <core/wdt.h>
#include <drivers/serial.h>
#include <logic.h>
#include <tools/common.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef CONFIG_SYSTICK_TEST
static bool test_systick(void);
#endif

#ifdef CONFIG_SYSTICK_TEST
static bool test_systick(void)
{
	unsigned int delays[] = {1, 2, 5, 9, 10, 25, 35, 50, 100, 1000, 5000};
	size_t i;
	uint64_t delta;
	uint32_t delta_ms;
	struct systick_time t1, t2;

	for (i = 0; i < ARRAY_SIZE(delays); i++) {
		pr_info("%zu: %u\n", i, delays[i]);
		systick_get_time(&t1);
		mdelay(delays[i]);
		systick_get_time(&t2);
		delta = systick_calc_diff(&t1, &t2);
		delta_ms = delta / 1000000UL;
		if (delays[i] != delta_ms)
			goto err_1;
	}

	pr_info("[SUCCESS]\n");
	return true;

err_1:
	pr_info("[FAIL]\n");
	pr_info("Test data: %u msec\n", delays[i]);
	pr_info("Calculated data(nsec): %llu\n", delta);
	pr_info("Calculated data(msec): %lu\n", delta_ms);
	return false;
}
#endif /* CONFIG_SYSTICK_TEST */

static void init_reset(void)
{
	pr_info("Reboot reason: %s\n", reset_cause_name(reset_cause()));
	reset_clear();
}

static void init_core(void)
{
	int err;

	const struct swtimer_hw_tim hw_tim = {
		.base = SWTIMER_TIM_BASE,
		.irq = SWTIMER_TIM_IRQ,
		.rst = SWTIMER_TIM_RST,
		.arr = SWTIMER_TIM_ARR_VAL,
		.psc = SWTIMER_TIM_PSC_VAL,
	};
	struct serial_params serial = {
		.uart = SERIAL_USART,
		.baud = CONFIG_SERIAL_SPEED,
		.bits = 8,
		.stopbits = USART_STOPBITS_1,
		.parity = USART_PARITY_NONE,
		.mode = USART_MODE_TX_RX,
		.flow_control = USART_FLOWCONTROL_NONE
	};

	irq_init();
#ifndef CONFIG_SYSTICK_TEST
	err = wdt_init();
	if (err) {
		pr_err("Can't initialize watchdog timer: %d\n", err);
		hang();
	}
#endif /* CONFIG_SYSTICK_TEST */
	err = systick_init();
	if (err) {
		pr_err("Error: Can't initialize systick: %d\n", err);
		hang();
	}

	board_init();
	serial_init(&serial);
	init_reset();
	sched_init();

	err = swtimer_init(&hw_tim);
	if (err) {
		pr_emerg("Error: Can't initialize swtimer\n");
		hang();
	}
}

int main(void)
{
	init_core();

#ifdef CONFIG_SYSTICK_TEST
	bool res;

	res = test_systick();
	if (!res)
		hang();
#else
	logic_start();
#endif /* CONFIG_SYSTICK_TEST */

	sched_start();

}
