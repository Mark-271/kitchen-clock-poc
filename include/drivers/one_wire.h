/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 *         Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef DRIVERS_ONE_WIRE_H
#define DRIVERS_ONE_WIRE_H

#include <stdint.h>

struct ow {
	uint32_t port;
	uint16_t pin;
};

int ow_init(struct ow *obj);
void ow_exit(struct ow *obj);
int ow_reset_pulse(struct ow *obj);
void ow_write_byte(struct ow *obj, uint8_t byte);
int8_t ow_read_byte(struct ow *obj);

#endif /* DRIVERS_ONE_WIRE_H */
