// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <tools/melody.h>

#define MELODY_LEN 11 /* number of notes in melody */

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
#define NO_TONE		0

/* Simple melody */
struct note mel[] = {
	{ D4, QUAVER },
	{ A5, CROTCHET },
	{ A4, QUAVER },
	{ B5, QUAVER },
	{ C5, QUAVER },
	{ B4, QUAVER },
	{ NO_TONE, QUAVER },
	{ C5, CROTCHET },
	{ D5, QUAVER },
	{ A5, QUAVER },
	{ D4, MINIM },
};

struct theme theme = {
	.mlen = MELODY_LEN,
	.melody = mel,
};
