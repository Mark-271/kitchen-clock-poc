#include <board.h>
#include <core/irq.h>
#include <core/log.h>
#include <core/sched.h>
#include <core/swtimer.h>
#include <logic.h>
#include <tools/common.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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

	irq_init();
	board_init();
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
