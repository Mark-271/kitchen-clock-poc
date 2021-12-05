/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 */

#ifndef CONFIG_RELEASE_H
#define CONFIG_RELEASE_H

#include "config_common.h"

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

#endif /* CONFIG_RELEASE_H */
