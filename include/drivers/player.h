/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef DRIVERS_PLAYER
#define DRIVERS_PLAYER

#include <tools/melody.h>
#include <stdint.h>

typedef void (*player_cb_1_t)(uint16_t tone, uint16_t duration);
typedef void (*player_cb_2_t)(void);

struct sound_track {
	const struct note *melody;
	size_t melody_len;
	size_t pos; /* indicates what note to play next */
};

struct player {
	player_cb_1_t cb1;
	player_cb_2_t cb2;
	struct sound_track strack;
};

int player_init(struct player *obj, const struct note *melody,
		size_t melody_len, player_cb_1_t cb1, player_cb_2_t cb2);
void player_exit(struct player *obj);
void player_play_next_note(struct player *obj);
void player_stop(struct player *obj);

#endif /* DRIVERS_PLAYER */
