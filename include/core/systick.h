/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef CORE_SYSTICK_H
#define CORE_SYSTICK_H

#include <stdint.h>

#define NSEC_PER_SEC		1000000000UL

struct systick_time {
	uint32_t sec;
	uint32_t nsec;
};

int systick_init(void);
void systick_exit(void);
void systick_get_time (struct systick_time *t);
uint64_t systick_calc_diff(const struct systick_time *t1,
			   const struct systick_time *t2);

#endif /* CORE_SYSTICK_H */
