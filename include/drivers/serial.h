/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 */

#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

#include <tools/common.h>
#include <stdint.h>

struct serial_params {
	uint32_t uart;
	uint32_t baud;
	uint32_t bits;
	uint32_t stopbits;
	uint32_t parity;
	uint32_t mode;
	uint32_t flow_control;
};

#ifdef CONFIG_SERIAL_CONSOLE
int serial_init(struct serial_params *params);
void serial_exit(void);
#else
static inline int serial_init(struct serial_params *params)
{
	UNUSED(params);
	return 0;
}

static inline void serial_exit(void)
{
}
#endif /* CONFIG_SERIAL_CONSOLE*/

#endif /* DRIVERS_SERIAL_H */
