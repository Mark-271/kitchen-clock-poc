// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <core/systick.h>
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
#define BPM		120		/* Bits per minute */
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
static void melody_play_tune(struct buzz *obj)
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
	}

	/* Delay with max value of 1 sec */
	for (i = 0; i < 10; i++)
		mdelay(100);
}

void melody_stop_tune(struct buzz *obj)
{
	buzz_notune(obj);
}

/**
 * Play theme for the specified time.
 *
 * @param obj Buzzer object
 * @param period Time in milliseconds to reproduce a melody
 */
void melody_play_theme(struct buzz *obj, unsigned int period)
{
	struct systick_time _t1, _t2;

	systick_get_time(&_t1);
	while (1) {
		systick_get_time(&_t2);
		unsigned int _elapsed = systick_calc_diff(&_t1, &_t2);
		_elapsed /= 1000000UL;

		melody_play_tune(obj);

		if (_elapsed > period)
			break;
	}
}
