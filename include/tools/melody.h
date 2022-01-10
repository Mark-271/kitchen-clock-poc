/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef TOOLS_MELODY_H
#define TOOLS_MELODY_H

#include <drivers/buzzer.h>

void melody_play_theme(struct buzz *obj, unsigned int period);
void melody_stop_tune(struct buzz *obj);

#endif /* TOOLS_MELODY_H */
