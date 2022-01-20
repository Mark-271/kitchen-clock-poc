/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef TOOLS_MELODY_H
#define TOOLS_MELODY_H

struct note {
	int tone;
	int duration;
};

struct theme {
	int mlen;
	struct note *melody;
};

extern struct theme theme;

#endif /* TOOLS_MELODY_H */
