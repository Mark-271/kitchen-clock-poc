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
#include <stdint.h>
#include <stdio.h>

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
