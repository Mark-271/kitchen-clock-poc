#include <core/log.h>
#include <core/sched.h>
#include <core/systick.h>
#include <core/swtimer.h>
#include <string.h>

struct task {
	const char *name;		/* task name */
	task_func_t func;		/* task function to run */
	void *data;			/* user private data; passed to func */
#ifdef CONFIG_SCHED_PROFILE
	uint32_t sec;
	uint32_t nsec;
#endif /* CONFIG_SCHED_PROFILE */
};

/* Contains status of each task (1 - data is ready; 0 - blocked) */
static uint32_t sched_ready;
static struct task task_list[TASK_NR];
static int current;			/* current pos in task_list[] */

#ifdef CONFIG_SCHED_PROFILE
#define SCHED_PROFILER_PERIOD		5000	/* msec */
/* 0 - collect statistics for the whole boot; 1 - for SCHED_PROFILER_PERIOD */
#define SCHED_PROFILER_ITERATIVE	0
/* Total execution time, including scheduler routines (sec + nsec) */
static uint32_t profiler_total_nsec;
static uint32_t profiler_total_sec;
static uint32_t idle_sec;
static uint32_t idle_nsec;

static struct swtimer_sw_tim swtim;
#endif /* CONFIG_SCHED_PROFILE */

#ifdef CONFIG_SCHED_PROFILE

static inline __attribute__((always_inline)) void sched_profile_print(const char *task, int perc)
{
	printk("%s : %d%%\n", task, perc);
}

static void sched_profile_timer_tick(void *data)
{
	uint64_t tasks_ns = 0;
	uint64_t total_ns;
	uint64_t sched_ns, idle_ns;
	int sched_perc, idle_perc;
	int i;

	UNUSED(data);

	for (i = 0; i < TASK_NR; ++i) {
		tasks_ns += (uint64_t)task_list[i].sec * NSEC_PER_SEC;
		tasks_ns += (uint64_t)task_list[i].nsec;
	}

	total_ns = (uint64_t)profiler_total_sec * NSEC_PER_SEC +
		   profiler_total_nsec;
	idle_ns = (uint64_t)idle_sec * NSEC_PER_SEC + idle_nsec;
	idle_perc = (100ULL * idle_ns) / total_ns;
	sched_ns = total_ns - (tasks_ns + idle_ns);
	sched_perc = (100ULL * sched_ns) / total_ns;

	printk("\nScheduler profiler:\n");
	sched_profile_print("sched + IRQs", sched_perc);
	sched_profile_print("idle", idle_perc);
	for (i = 0; i < TASK_NR; ++i) {
		uint64_t task_ns;
		int task_perc;

		if (!task_list[i].func)
			continue;

		task_ns = (uint64_t)task_list[i].sec * NSEC_PER_SEC +
			  task_list[i].nsec;
		task_perc = (100ULL * task_ns) / total_ns;
#if SCHED_PROFILER_ITERATIVE == 1
		task_list[i].sec = 0;
		task_list[i].nsec = 0;
#endif

		sched_profile_print(task_list[i].name, task_perc);
	}

#if SCHED_PROFILER_ITERATIVE == 1
	profiler_total_sec = 0;
	profiler_total_nsec = 0;
	idle_sec = 0;
	idle_nsec = 0;
#endif
}

static int sched_profile_init(void)
{
	int timer_id;

	swtim.cb = sched_profile_timer_tick;
	swtim.period = SCHED_PROFILER_PERIOD;
	swtim.data = &swtim;

	timer_id = swtimer_tim_register(&swtim);
	if (timer_id < 0)
		return timer_id;

	return 0;
}

#else /* !CONFIG_SCHED_PROFILE */

static inline int sched_profile_init(void) { return 0; }

#endif /* CONFIG_SCHED_PROFILE */

/**
 * Set "Blocked" state for specified task (waiting for new data).
 *
 * @param task_id Task ID, starting from 0
 */
static void sched_set_blocked(int task_id)
{
	unsigned long flags;

	enter_critical(flags);
	WRITE_ONCE(sched_ready, sched_ready & ~BIT(task_id));
	exit_critical(flags);
}

static int sched_find_empty_slot(void)
{
	int i;

	for (i = 0; i < TASK_NR; ++i) {
		if (!task_list[i].func)
			return i;
	}

	return -1;
}

static int sched_slot_by_name(const char *name)
{
	int i;

	for (i = 0; i < TASK_NR; ++i) {
		if (strcmp(name, task_list[i].name) == 0)
			return i;
	}

	return -1;
}

/**
 * Look for the next ready to run task.
 *
 * @return Index of task or -1 if no tasks found
 */
static int sched_find_next(void)
{
	uint32_t tasks_ready = READ_ONCE(sched_ready);
	int i;

	/*
	 * Iterate over all tasks, including current, but starting from next one
	 * (fair scheduling).
	 */
	for (i = current + 1; i <= current + TASK_NR; ++i) {
		const int idx = i % TASK_NR;
		int ready = tasks_ready & (1 << idx);

		if (ready)
			return idx;
	}

	return -1;
}

#ifdef CONFIG_SCHED_IDLE
/**
 * "Idle" task function.
 *
 * This function must be called with interrupts disabled. Best way is to disable
 * interrupts before checking if all tasks are blocked (waiting for new data
 * from some interrupt). This way one can avoid a race condition when new data
 * arrived after the check but before WFI, and we go to sleep when some task
 * is actually not blocked.
 *
 * WFI instruction will wake up the CPU on interrupt, even if interrupts are
 * disabled. If there is some pending interrupt when calling WFI, it won't go to
 * sleep, performing NOP instead.
 */
static inline __attribute__((always_inline)) void sched_idle(void)
{
#ifdef CONFIG_SCHED_PROFILE
	struct systick_time t1, t2;
	uint64_t diff_ns;

	systick_get_time(&t1);
#endif
	/*
	 * DSB: follow AN321 guidelines for WFI
	 * ISB: without this "if (!sched_ready)" can finish after WFI, which
	 *      leads to complete system stuck when no new IRQ happen
	 * WFI: CPU sleep; improves power management
	 * ISB: just mimic what FreeRTOS does
	 */
	__asm__ ("dsb");
	__asm__ ("isb");
	__asm__ ("wfi");
	__asm__ ("isb");

#ifdef CONFIG_SCHED_PROFILE
	systick_get_time(&t2);
	diff_ns = systick_calc_diff(&t1, &t2);
	idle_nsec += (uint32_t)diff_ns; /* XXX: Check integer promotion */
	while (idle_nsec > NSEC_PER_SEC) {
		idle_nsec -= NSEC_PER_SEC;
		idle_sec++;
	}
#endif
}
#else
static inline void sched_idle(void)
{
}
#endif

/**
 * Run next task from scheduler list.
 *
 * @return Index of executed task or -1 if no task was executed
 */
static int sched_run_next(void)
{
	unsigned long irq_flags;
	int next;
#ifdef CONFIG_SCHED_PROFILE
	struct systick_time t1, t2;
	uint64_t diff_ns;
#endif

	enter_critical(irq_flags);
	if (!READ_ONCE(sched_ready)) {
		sched_idle();
		exit_critical(irq_flags);
		return -1;
	}
	exit_critical(irq_flags);

	next = sched_find_next();
	if (next == -1)
		return -1;
	current = next;

#ifdef CONFIG_SCHED_PROFILE
	systick_get_time(&t1);
#endif

	/*
	 * Clear the task flag (put it in "Blocked" state) before running task
	 * function, to avoid race conditions.
	 */
	sched_set_blocked(current);
	task_list[current].func(task_list[current].data);

#ifdef CONFIG_SCHED_PROFILE
	systick_get_time(&t2);
	diff_ns = systick_calc_diff(&t1, &t2);
	task_list[current].nsec += diff_ns;
	while (task_list[current].nsec > NSEC_PER_SEC) {
		task_list[current].nsec -= NSEC_PER_SEC;
		task_list[current].sec++;
	}
#endif

	return current;
}

/**
 * Initialize the scheduler.
 *
 * @return 0 on success or negative value on error
 */
int sched_init(void)
{
	int res;

	res = sched_profile_init();
	if (res)
		return res;

	return res;
}

/**
 * Initiate the heart beat.
 *
 * Run the super loop, which executes registered tasks.
 *
 * @note Does not return.
 */
int sched_start(void)
{
	for (;;) {
#ifdef CONFIG_SCHED_PROFILE
		struct systick_time t1, t2;
		uint64_t diff_ns;

		systick_get_time(&t1);
#endif

		sched_run_next();

#ifdef CONFIG_SCHED_PROFILE
		systick_get_time(&t2);
		diff_ns = systick_calc_diff(&t1, &t2);
		profiler_total_nsec += diff_ns;
		while (profiler_total_nsec > NSEC_PER_SEC) {
			profiler_total_nsec -= NSEC_PER_SEC;
			profiler_total_sec++;
		}
#endif
	}
}

/**
 * Add new scheduler task.
 *
 * Create new task and add it to scheduler list. Created task will have
 * "Blocked" state by default, and will be only executed when new data arrives
 * for this task.
 *
 * When new data is available, use @ref sched_set_ready() to let scheduler know
 * it should run corresponding task. After running the task, scheduler sets its
 * state to "Blocked" automatically.
 *
 * @param name Task name, must be unique
 * @param func Task function (callback) to be executed in schedule loop
 * @param data Pointer to data to be passed to task function
 * @param task_id If not null, will contain ID of created task, starting from 1
 * @return 0 on success or negative value on error
 *
 * @note This function can be called before sched_init() and sched_start()
 */
int sched_add_task(const char *name, task_func_t func, void *data,
		   int *task_id)
{
	int slot;		/* next empty slot index for new task */

	slot = sched_find_empty_slot();
	if (slot < 0)
		return slot;
	if (sched_slot_by_name(name) >= 0)
		return -1;
	if (strlen(name) == 0)
		return -2;
	if (func == NULL)
		return -3;

	/* Add new task to task list */
	memset(&task_list[slot], 0, sizeof(struct task));
	task_list[slot].name = name;
	task_list[slot].func = func;
	task_list[slot].data = data;

	if (task_id)
		*task_id = slot + 1;

	return 0;
}

/**
 * Remove task.
 *
 * @param task_id Task ID obtained on sched_add_task()
 * @return 0 on success or negative value on error
 */
int sched_del_task(int task_id)
{
	int idx = task_id - 1;

	cm3_assert(idx >= 0 && idx < TASK_NR);

	if (!task_list[idx].func)
		return -1;

	sched_set_blocked(idx);
	memset(&task_list[idx], 0, sizeof(struct task));

	return 0;
}

/**
 * Set "Ready" state for specified task (new data is available).
 *
 * Report to scheduler that task has new data and ready to be executed.
 *
 * @param task_id Task ID (obtained in sched_add_task())
 */
void sched_set_ready(int task_id)
{
	unsigned long flags;

	enter_critical(flags);
	WRITE_ONCE(sched_ready, sched_ready | BIT(task_id - 1));
	exit_critical(flags);
}
