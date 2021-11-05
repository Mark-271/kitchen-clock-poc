/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef CONFIG_RELEASE_H
#define CONFIG_RELEASE_H

/* ---- Serial console ---- */
/* Enable serial console */
/*#define CONFIG_SERIAL_CONSOLE */
/* Serial console baud rate */
#define CONFIG_SERIAL_SPEED		115200
/* Catch sbrk() syscall when CONFIG_SERIAL_CONSOLE is disabled */
#define CONFIG_SERIAL_SBRK_TRAP

/* ---- Logging system ---- */
/* Messages with level less than this will be printed to the console; 1-8 */
#define CONFIG_LOGLEVEL			1

/* Vector table size (=sizeof(vector_table) */
#define CONFIG_VTOR_SIZE		0x150

/* GPIO level transient time, usec */
#define CONFIG_GPIO_STAB_DELAY		10

/* ---- Watchdog timer ---- */
/* Enable watchdog timer */
#define CONFIG_WDT
/* Watchdog timer period, msec */
#define CONFIG_WDT_PERIOD		1000

/* ---- "Scheduler" ---- */
/* Enable CPU sleeping when no tasks are awaiting execution */
#define CONFIG_SCHED_IDLE

#endif /* CONFIG_RELEASE_H */
