/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef TOOLS_MELODY_H
#define TOOLS_MELODY_H

#include <drivers/buzzer.h>

void melody_play(struct buzz *obj);
void melody_stop(struct buzz *obj);

#endif /* TOOLS_MELODY_H */
