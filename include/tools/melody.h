/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef TOOLS_MELODY_H
#define TOOLS_MELODY_H

#include <stddef.h>

struct note {
	int tone;
	int duration;
};

extern const struct note melody_alarm[];
extern const size_t melody_alarm_len;

#endif /* TOOLS_MELODY_H */
