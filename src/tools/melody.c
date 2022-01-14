// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <core/systick.h>
#include <core/wdt.h>
#include <tools/common.h>
#include <tools/melody.h>
#include <stddef.h>

/* The note frequency given in Hz */
#define A4		440
#define A5		880
#define B4		494
#define B5		988
#define C5		523
#define D4		294
#define D5		587

/* Note duration in msec */
#define BPM		480		/* Bits per minute */
#define CROTCHET	(60000 / BPM)	/* Quarter note */
#define QUAVER		(CROTCHET / 2)	/* Eighth note */
#define MINIM		(CROTCHET * 2)	/* Half note */

/* Simple melody */
static uint16_t theme[] = {
	D4, A5, A4, B5, C5, B4, 0,
	C5, D5, A4, D4,
};

static int tempo[] = {
	QUAVER, CROTCHET, QUAVER, QUAVER, QUAVER, QUAVER, QUAVER,
	CROTCHET, QUAVER, QUAVER, MINIM,
};

/**
 * Play tune.
 *
 * @param obj Buzzer object
 */
void melody_play_tune(struct buzz *obj)
{
	unsigned size = ARRAY_SIZE(theme);
	size_t i;
	uint16_t pause;
	unsigned long flags;

	for (i = 0; i < size; i++) {
		pause = tempo[i] + tempo[i] / 3;
		buzz_tune(obj, theme[i], tempo[i]);
		enter_critical(flags);
		mdelay(pause);
		exit_critical(flags);
		wdt_reset();
	}
}

void melody_stop_tune(struct buzz *obj)
{
	buzz_notune(obj);
}
