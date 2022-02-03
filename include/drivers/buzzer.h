/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef DRIVERS_BUZZER_H
#define DRIVERS_BUZZER_H

#include <stdint.h>

struct buzz {
	uint32_t port;
	uint16_t pin;
};

int buzzer_init(struct buzz *obj, uint32_t gpio_port, uint16_t gpio_pin);
void buzzer_exit(struct buzz *obj);
void buzzer_make_sound(struct buzz *obj, uint16_t freq, uint16_t duration);
void buzzer_stop_sound(struct buzz *obj);

#endif /*DRIVERS_BUZZER_H */
