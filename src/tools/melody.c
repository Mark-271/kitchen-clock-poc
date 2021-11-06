// SPDX-License-Identifier: GPL-3.0-or-later

#include <tools/melody.h>
#include <tools/common.h>
#include <tools/pitches.h>

/* Mario Underworld melody */
static uint16_t theme[] = {
	NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
	NOTE_AS3, NOTE_AS4, 0,
	0,
	NOTE_C4, NOTE_C5, NOTE_A3, NOTE_A4,
	NOTE_AS3, NOTE_AS4, 0,
	0,
	NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
	NOTE_DS3, NOTE_DS4, 0,
	0,
	NOTE_F3, NOTE_F4, NOTE_D3, NOTE_D4,
	NOTE_DS3, NOTE_DS4, 0,
	0, NOTE_DS4, NOTE_CS4, NOTE_D4,
	NOTE_CS4, NOTE_DS4,
	NOTE_DS4, NOTE_GS3,
	NOTE_G3, NOTE_CS4,
	NOTE_C4, NOTE_FS4,NOTE_F4, NOTE_E3, NOTE_AS4, NOTE_A4,
	NOTE_GS4, NOTE_DS4, NOTE_B3,
	NOTE_AS3, NOTE_A3, NOTE_GS3,
	0, 0, 0
};

/* Mario Underworld tempo */
static int tempo[] = {
	12, 12, 12, 12,
	12, 12, 6,
	3,
	12, 12, 12, 12,
	12, 12, 6,
	3,
	12, 12, 12, 12,
	12, 12, 6,
	3,
	12, 12, 12, 12,
	12, 12, 6,
	6, 18, 18, 18,
	6, 6,
	6, 6,
	6, 6,
	18, 18, 18,18, 18, 18,
	10, 10, 10,
	10, 10, 10,
	3, 3, 3
};

/**
 * Sing  melody.
 *
 * @param obj Buzzer object
 */
void melody_play(struct buzz *obj)
{
	int size = ARRAY_SIZE(theme);
	int i;
	uint16_t duration;
	uint16_t note_pause;
	unsigned long flags;

	for (i = 0; i < size; i++) {
		duration = 1000 / tempo[i];
		note_pause = duration + duration / 3;
		buzz_tune(obj, theme[i], duration);
		enter_critical(flags);
		mdelay(note_pause);
		exit_critical(flags);
	}
}

void melody_stop(struct buzz *obj)
{
	buzz_notune(obj);
}
