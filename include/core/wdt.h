/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef CORE_WDT_H
#define CORE_WDT_H

#include <tools/common.h>

#ifdef CONFIG_WDT

int wdt_init(void);
void wdt_reset(void);
int wdt_task_add(const char *name);
int wdt_task_del(int id);
void wdt_task_report(int id);

#else

static inline int wdt_init(void) { return 0; }
static inline void wdt_reset(void) {}
static inline int wdt_task_add(const char *name) { UNUSED(name); return 0; }
static inline int wdt_task_del(int id) { UNUSED(id); return 0; }
static inline void wdt_task_report(int id) { UNUSED(id); }

#endif /* CONFIG_WDT */

#endif /* CORE_WDT_H */
