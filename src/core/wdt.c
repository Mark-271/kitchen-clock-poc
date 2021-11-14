// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 */

/**
 * @file
 *
 * Watchdog Timer framework.
 *
 * Design of watchdog timer for reliable systems must account for each
 * periodic task execution. In other words, we should only kick watchdog
 * when all periodic tasks executed properly. This framework provides such an
 * API, so that each periodic task can register itself here, and then report
 * successful task execution. Once all periodic tasks reported successful
 * execution, this framework resets watchdog timer automatically.
 *
 * Although it could be done in sched.c, it would be a bad design
 * decision, as it might be desirable to only kick watchdog when task was
 * executed *successfully*, and sometimes task might become not periodical.
 * So the best approach is to use this new separate WDT framework to track
 * all periodic tasks execution and kick watchdog when in happens. If no
 * periodic task is registered in WDT framework, watchdog timer won't be
 * kicked at all, and CPU will reboot. Hence we presume there always must
 * be at least one period task running in our system, otherwise system is
 * considered stuck.
 *
 * The watchdog will reset CPU in next cases:
 *   1. wdt internal task doesn't run (e.g. due to some bug in scheduler)
 *   2. wdt internal task can't be run due to all other tasks are starving CPU
 *      (wdt task has lowest priority)
 *   3. if any of registered periodic tasks doesn't run
 *
 * This is probably the best sub-optimal approach right now. More paranoid
 * approach would be to check that each periodic task took estimated period
 * of time to finish, but what's done should be enough to assure the safety
 * of the system.
 */

#ifdef CONFIG_WDT

#include <core/wdt.h>
#include <core/sched.h>
#include <libopencm3/stm32/iwdg.h>
#include <string.h>

#define MAX_WDT_TASKS	32

struct wdt {
	const char *task_names[MAX_WDT_TASKS];	/* registered tasks names */
	uint32_t tasks;				/* registered tasks mask */
	uint32_t executed_tasks;		/* executed tasks mask */
	int tid;				/* WDT task ID */
};

static const char * const wdt_task_name = "wdt_task";
static struct wdt wdt;

static void wdt_task(void *data)
{
	UNUSED(data);

	/* Ignore the case when some extra bits are set in executed_tasks */
	if ((wdt.tasks & wdt.executed_tasks) == wdt.tasks) {
		wdt_reset();
		wdt.executed_tasks = 0;
	}
}

/**
 * Initialize the watchdog timer and prepare per-task functionality.
 *
 * @return 0 on success or negative value on error
 */
int wdt_init(void)
{
	iwdg_set_period_ms(CONFIG_WDT_PERIOD);
	iwdg_start();

	return sched_add_task(wdt_task_name, wdt_task, NULL, &wdt.tid);
}

/**
 * Explicitly kick the watchdog.
 *
 * It's better to use per-task API (wdt_task_* functions).
 */
void wdt_reset(void)
{
	iwdg_reset();
}

/**
 * Register periodic task that should contribute to watchdog reset.
 *
 * @param name Task name; must be unique
 * @return WDT task ID (starting from 1) or negative value on error
 */
int wdt_task_add(const char *name)
{
	size_t i;
	int idx = -1;

	cm3_assert(strlen(name) > 0);

	for (i = 0; i < MAX_WDT_TASKS; ++i) {
		if (wdt.task_names[i] != NULL) {
			if (strcmp(name, wdt.task_names[i]) == 0)
				return -1;
			continue;
		}

		if (idx == -1)
			idx = i;
	}

	if (idx == -1)
		return -2;

	wdt.task_names[idx] = name;
	wdt.tasks |= BIT(idx);
	wdt.executed_tasks &= ~BIT(idx);

	return idx + 1;
}

/**
 * Unregister task from WDT list.
 *
 * @param id WDT task ID
 * @return 0 on success or negative value on error
 */
int wdt_task_del(int id)
{
	int idx = id - 1;

	cm3_assert(idx >= 0 && idx < MAX_WDT_TASKS);

	if (wdt.task_names[idx] == NULL)
		return -1;

	wdt.task_names[idx] = NULL;
	wdt.tasks &= ~BIT(idx);
	wdt.executed_tasks &= ~BIT(idx);

	return 0;
}

/**
 * Report specific task to be successfully executed.
 *
 * When all registered tasks are executed, watchdog timer will be reset.
 *
 * @param id WDT task ID
 */
void wdt_task_report(int id)
{
	int idx = id - 1;

	cm3_assert(idx >= 0 && idx < MAX_WDT_TASKS);
	cm3_assert(wdt.task_names[idx] != NULL);

	wdt.executed_tasks |= BIT(idx);
	sched_set_ready(wdt.tid);
}

#endif /* CONFIG_WDT */
