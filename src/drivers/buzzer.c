// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <drivers/buzzer.h>
#include <tools/common.h>
#include <libopencm3/stm32/gpio.h>
#include <stddef.h>
#include <string.h>

void buzzer_stop_sound(struct buzzer *obj)
{
	gpio_clear(obj->port, obj->pin);
}

/**
 * Reproduce musical note.
 *
 * @param obj Buzzer object
 * @param freq Tune frequency to reproduce
 * @param duration Play time given in miliseconds
 */
void buzzer_make_sound(struct buzzer *obj, uint16_t freq, uint16_t duration)
{
	uint16_t cycles = freq * duration / 1000;
	uint16_t pause = 1000 / freq;
	size_t i;

	for (i = 0; i < cycles; i++) {
		gpio_toggle(obj->port, obj->pin);
		mdelay(pause);
	}

	gpio_clear(obj->port, obj->pin);
}

int buzzer_init(struct buzzer *obj, uint32_t gpio_port, uint16_t gpio_pin)
{
	obj->port = gpio_port;
	obj->pin = gpio_pin;

	gpio_set(obj->port, obj->pin);

	return 0;
}

void buzzer_exit(struct buzzer *obj)
{
	memset(obj, 0, sizeof(*obj));
}
