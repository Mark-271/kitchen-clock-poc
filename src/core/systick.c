#include <core/systick.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/rcc.h>

#define SYSTICK_FREQ		0x3e8 /* 1000 overflows per second */
#define SYSTICK_CNT_MASK	0xffffff

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
