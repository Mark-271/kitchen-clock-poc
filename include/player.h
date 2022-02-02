/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef PLAYER_H
#define PLAYER_H

#include <tools/melody.h>
#include <stdint.h>

typedef void (*player_play_note_cb_t)(uint16_t tone, uint16_t duration);

struct player {
	player_play_note_cb_t play_note_cb;
	const struct note *melody;
	size_t melody_len;
	size_t pos; /* indicates what note to play next */
};

int player_init(struct player *obj, const struct note *melody,
		size_t melody_len, player_play_note_cb_t play_note_cb);
void player_exit(struct player *obj);
void player_play_next_note(struct player *obj);
void player_stop(struct player *obj);

#endif /* PLAYER_H */
