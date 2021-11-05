/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef CORE_LOG_H
#define CORE_LOG_H

#include <tools/common.h>
#include <stdio.h>

enum log_level {
	LOG_EMERG	= 0, /* system is unstable */
	LOG_ALERT	= 1, /* action must be taken immediately */
	LOG_CRIT	= 2, /* critical conditions */
	LOG_ERR		= 3, /* error that prevents something from working */
	LOG_WARNING	= 4, /* warning may prevent optimal operation */
	LOG_NOTICE	= 5, /* normal but significant condition, printk() */
	LOG_INFO	= 6, /* general information message */
	LOG_DEBUG	= 7, /* basic debug-level message */
};

/*
 * Dummy printk for disabled debugging statements to use whilst maintaining
 * gcc's format and side-effect checking.
 */
#define no_printk(fmt, ...)				\
({							\
	if (0)						\
		printf(fmt, ##__VA_ARGS__);		\
	0;						\
})

#ifdef CONFIG_SERIAL_CONSOLE
#define printk(fmt, ...)	printf(fmt, ##__VA_ARGS__)
#else
#define printk(fmt, ...)	no_printk(fmt, ##__VA_ARGS__)
#endif

#define __printk(level, fmt, ...)					\
({									\
	level < CONFIG_LOGLEVEL ? printk(fmt, ##__VA_ARGS__) : 0;	\
})

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define pr_emerg(fmt, ...) \
	__printk(LOG_EMERG, pr_fmt(fmt), ##__VA_ARGS__)
#define pr_alert(fmt, ...) \
	__printk(LOG_ALERT, pr_fmt(fmt), ##__VA_ARGS__)
#define pr_crit(fmt, ...) \
	__printk(LOG_CRIT, pr_fmt(fmt), ##__VA_ARGS__)
#define pr_err(fmt, ...) \
	__printk(LOG_ERR, pr_fmt(fmt), ##__VA_ARGS__)
#define pr_warn(fmt, ...) \
	__printk(LOG_WARNING, pr_fmt(fmt), ##__VA_ARGS__)
#define pr_notice(fmt, ...) \
	__printk(LOG_NOTICE, pr_fmt(fmt), ##__VA_ARGS__)
#define pr_info(fmt, ...) \
	__printk(LOG_INFO, pr_fmt(fmt), ##__VA_ARGS__)

/* pr_debug() should produce zero code unless DEBUG is defined */
#ifdef DEBUG
#define pr_debug(fmt, ...) \
	__printk(LOG_DEBUG, pr_fmt(fmt), ##__VA_ARGS__)
#else
#define pr_debug(fmt, ...) \
	no_printk(pr_fmt(fmt), ##__VA_ARGS__)
#endif

#endif /* CORE_LOG_H */
