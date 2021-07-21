#include <drivers/buzzer.h>
#include <tools/common.h>
#include <libopencm3/stm32/gpio.h>
#include <stddef.h>

/**
 * Reproduce musical note.
 *
 * @param obj Buzzer object
 * @param freq Tune frequency to reproduce
 * @param duration Play time given in miliseconds
 */
void buzz_tune(struct buzz *obj, uint16_t freq, uint16_t duration)
{
	unsigned long flags;
	uint16_t cycles = freq * duration / 1000;
	uint16_t pause = 1000 / freq;
	size_t i;

	for (i = 0; i < cycles; i++) {
		gpio_toggle(obj->port, obj->pin);
		enter_critical(flags);
		mdelay(pause);
		exit_critical(flags);
	}
}

void buzz_notune(struct buzz *obj)
{
	gpio_clear(obj->port, obj->pin);
}

int buzz_init(struct buzz *obj, uint32_t gpio_port, uint16_t gpio_pin)
{
	obj->port = gpio_port;
	obj->pin = gpio_pin;

	gpio_set(obj->port, obj->pin);

	return 0;
}

void buzz_exit(struct buzz *obj)
{
	UNUSED(obj);
}