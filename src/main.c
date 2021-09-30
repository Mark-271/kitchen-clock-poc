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
bool test_systick(void);
#endif

#ifdef CONFIG_SYSTICK_TEST
bool test_systick(void)
{
	uint64_t delays[] = {0, 1, 10, 100, 1000, 10000};
	size_t i;
	uint64_t delta;
	struct systick_time t1, t2;

	for (i = 0; i < ARRAY_SIZE(delays); i++) {
		systick_get_time(&t1);
		mdelay(delays[i]);
		systick_get_time(&t2);
		delta = systick_calc_diff(&t1, &t2);
		if (delays[i] != delta / 1000000)
			goto err_1;
	}

	for (i = 0; i < ARRAY_SIZE(delays); i++) {
		systick_get_time(&t1);
		udelay(delays[i]);
		systick_get_time(&t2);
		delta = systick_calc_diff(&t1, &t2);
		if (delays[i] != (delta / 1000))
			goto err_2;
	}

	pr_info("[SUCCESS]\n");
	return true;

err_1:
	pr_info("[FAIL]\n");
	pr_info("Test data: %llu msec\n", delays[i]);
	pr_info("Calculated data: %llu nsec\n", delta);
	return false;

err_2:
	pr_info("[FAIL]\n");
	pr_info("Test data: %llu usec\n", delays[i]);
	pr_info("Calculated data: %llu nsec\n", delta);
	return false;
}
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

	irq_init();
#if 0
	err = wdt_init();
	if (err) {
		pr_err("Can't initialize watchdog timer: %d\n", err);
		hang();
	}
#endif
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

	struct systick_time t1;
	struct systick_time t2;
	uint64_t delta;

	systick_get_time(&t1);
//	mdelay(1);
	udelay(1);
	systick_get_time(&t2);
	delta = systick_calc_diff(&t1, &t2);
	pr_info("delta(nsec) = %llu\n", delta);
	pr_info("delta(usec) = %llu\n", delta / 1000);
	pr_info("delta(msec) = %llu\n", delta / 1000000);
	pr_info("delta(sec) = %llu\n", delta / 1000000000);

	pr_info("------------\n");
	pr_info("t1->nsec(usec) = %lu(%lu)\n", t1.nsec, t1.nsec / 1000);
	pr_info("t2->nsec(usec) = %lu(%lu)\n", t2.nsec, t2.nsec / 1000);
#if 0
		if(!test_systick())
			hang();
#endif
#else

	logic_start();
	sched_start();

#endif /* CONFIG_SYSTICK_TEST */
}
