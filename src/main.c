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
#include <libopencm3/stm32/dbgmcu.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef DEBUG
/**
 * Configure clocks behavior in debug build.
 *
 * 1. Do not disable main CPU clock in SLEEP mode (WFI)
 * 2. Disable next timers when CPU core is halted (during debugging):
 *    - watchdog timer (IWDG)
 *    - software timer
 *
 * When using OpenOCD, the DBGMCU_CR register is overwritten with 0x307 value
 * in target/stm32f1x.cfg file, but let's write it here anyway, for another
 * GDB servers like st-util.
 */
static inline void dbg_init(void)
{
	DBGMCU_CR = DBGMCU_CR_SLEEP | DBGMCU_CR_STOP | DBGMCU_CR_STANDBY |
		    DBGMCU_CR_IWDG_STOP | DBGMCU_CR_WWDG_STOP |
		    SWTIMER_TIM_DBGMCU;
}
#else
static inline void dbg_init(void) { }
#endif

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

	dbg_init();
	irq_init();

	err = wdt_init();
	if (err) {
		pr_err("Can't initialize watchdog timer: %d\n", err);
		hang();
	}

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
	logic_start();
	sched_start();
}
