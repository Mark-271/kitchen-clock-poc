// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <drivers/player.h>
#include <tools/common.h>
#include <stddef.h>
#include <string.h>

/**
 * Play note.
 *
 * When function is called, plays one note and returns. Every
 * subsequent call plays following note of the melody.
 *
 * @param obj Device object
 */
void player_play_next_note(struct player *obj)
{
	if (obj->strack.pos > obj->strack.melody_len)
		obj->strack.pos = 0;

	obj->cb1((obj->strack.melody + obj->strack.pos)->tone,
		 (obj->strack.melody + obj->strack.pos)->duration);

	obj->strack.pos++;
}

/**
 * Stop playing melody.
 *
 * @param obj Device object
 */
void player_stop(struct player *obj)
{
	obj->strack.pos = 0;
	obj->cb2();
}

/**
 * Initialize player module.
 *
 * @param obj Device object
 * @param melody Music theme (array of notes) to play
 * @param melody_len Number of notes in @p melody array
 * @param cb1 Callback to call on demand
 * @param cb2 Callback to call on demand
 * @return 0
 */
int player_init(struct player *obj, const struct note *melody,
		size_t melody_len, player_cb_1_t cb1, player_cb_2_t cb2)
{
	obj->cb1 = cb1;
	obj->cb2 = cb2;
	obj->strack.melody = melody;
	obj->strack.melody_len = melody_len;
	obj->strack.pos = 0;

	return 0;
}

void player_exit(struct player *obj)
{
	memset(obj, 0, sizeof(*obj));
}
