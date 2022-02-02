// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <player.h>
#include <tools/common.h>
#include <stddef.h>
#include <string.h>

/**
 * Play note.
 *
 * When function is called, plays one note and returns. Every
 * subsequent call plays following note of the melody.
 *
 * @param obj Player object
 */
void player_play_next_note(struct player *obj)
{
	const struct note *note = &obj->melody[obj->pos];

	obj->play_note_cb(note->tone, note->duration);
	obj->pos++;
	obj->pos %= obj->melody_len;
}

/**
 * Stop playing melody.
 *
 * @param obj Player object
 */
void player_stop(struct player *obj)
{
	obj->pos = 0;
}

/**
 * Initialize player module.
 *
 * @param obj Player object
 * @param melody Music theme (array of notes) to play
 * @param melody_len Number of notes in @p melody array
 * @param play_note_cb Function to play note
 * @return 0 on success or negative number on error
 */
int player_init(struct player *obj, const struct note *melody,
		size_t melody_len, player_play_note_cb_t play_note_cb)
{
	obj->play_note_cb = play_note_cb;
	obj->melody = melody;
	obj->melody_len = melody_len;
	obj->pos = 0;

	return 0;
}

void player_exit(struct player *obj)
{
	memset(obj, 0, sizeof(*obj));
}
