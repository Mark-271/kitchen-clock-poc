#include <core/systick.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>

#define SYSTICK_FREQ		0x3e8 /* 1000 overflows per second */
#define SYSTICK_CNT_MASK	0xffffff

static volatile uint32_t ticks;

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
	ticks++;
}

/**
 * Initialize systick timer.
 *
 * @return 0 or -1 on error
 */
int systick_init(void)
{
	if (!systick_set_frequency(SYSTICK_FREQ, rcc_ahb_frequency))
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
