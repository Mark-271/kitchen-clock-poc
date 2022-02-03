/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef DRIVERS_BUZZER_H
#define DRIVERS_BUZZER_H

#include <stdint.h>

struct buzzer {
	uint32_t port;
	uint16_t pin;
};

int buzzer_init(struct buzzer *obj, uint32_t gpio_port, uint16_t gpio_pin);
void buzzer_exit(struct buzzer *obj);
void buzzer_make_sound(struct buzzer *obj, uint16_t freq, uint16_t duration);
void buzzer_stop_sound(struct buzzer *obj);

#endif /*DRIVERS_BUZZER_H */
