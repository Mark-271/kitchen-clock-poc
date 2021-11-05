/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef CORE_RESET_H
#define CORE_RESET_H

/* Possible STM32 system reset causes */
enum reset_cause {
	RESET_UNKNOWN,	/* unknown cause of reset */
	RESET_LPWR,	/* low power */
	RESET_WWDG,	/* window watchdog */
	RESET_IWDG,	/* independent watchdog */
	RESET_SOFT,	/* software reset */
	RESET_POR,	/* power-on / power-down */
	RESET_PIN,	/* external NRST pin */
};

enum reset_cause reset_cause(void);
const char *reset_cause_name(enum reset_cause cause);
void reset_clear(void);

#endif /* CORE_RESET_H */
