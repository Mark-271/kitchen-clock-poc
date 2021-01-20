#include <button.h>
#include <common.h>
#include <delay.h>
#include <libopencm3/stm32/gpio.h>
#include <stddef.h>

#define RELAXATION_TIME		10			/* ms */
#define RELAXING_DELAY		(RELAXATION_TIME / 10)	/* ms */

/**
 * Polling push button switch with considering possible contact bounce.
 *
 * The read data is considered valid only if there is no bouncing
 * for at least 10 msec.
 *
 * @param obj Button object
 * @return Read button state
 */
static uint16_t button_read_debounced_state(struct btn *obj)
{
	uint16_t val;
	unsigned long flags;
	size_t i = 0;

	enter_critical(flags);
	while (i < RELAXATION_TIME) {
		i++;
		val = gpio_get(obj->port, obj->pin);
		if (val)
			i--;
		mdelay(RELAXING_DELAY);
	}
	exit_critical(flags);

	return val;
}

/* Corresponding gpios are configured with internal pullup resistor */
int button_init(struct btn *obj)
{
	gpio_set(obj->port, obj->pin);

	return 0;
}

/* Destroy button obj */
void button_exit(struct btn *obj)
{
	UNUSED(obj);
}

/* Read button state */
uint16_t button_poll_input(struct btn *obj)
{
	return button_read_debounced_state(obj);
}

