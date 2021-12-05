/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 */

#ifndef CONFIG_COMMON_H
#define CONFIG_COMMON_H

/* Vector table size (=sizeof(vector_table) */
#define CONFIG_VTOR_SIZE		0x150

/* GPIO level transient time, usec */
#define CONFIG_GPIO_STAB_DELAY		10

/* ---- Watchdog timer ---- */
/* Enable watchdog timer */
#define CONFIG_WDT
/* Watchdog timer period, msec */
#define CONFIG_WDT_PERIOD		2000

/* ---- "Scheduler" ---- */
/* Enable CPU sleeping when no tasks are awaiting execution */
#define CONFIG_SCHED_IDLE
/* Enable profiler */
#define CONFIG_SCHED_PROFILE

#endif /* CONFIG_COMMON_H */
