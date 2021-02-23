/**
 * @file
 *
 * Software timers framework.
 *
 * Can be used by many users, but uses only one single hardware (general
 * purpose) timer underneath. Users can register software timer, and this
 * framework will call registered function when registered timeout expires.
 */

#include <core/swtimer.h>

#define SWTIMER_TIMERS_MAX	10
#define SWTIMER_TASK		"swtimer"

/* Software timer parameters */
struct swtimer_sw_tim {
	swtimer_callback_t cb;	/* function to call when this timer overflows */
	int period;		/* timer overflow period, msec */
	int remaining;		/* remaining time till overflow, msec */
	bool active;		/* if true, callback will be executed */
};

/* Driver struct (swtimer framework) */
struct swtimer {
	struct swtimer_hw_tim hw_tim;
	struct swtimer_sw_tim timer_list[SWTIMER_TIMERS_MAX];
	volatile int ticks;		/* global ticks counter */
	int task_id;			/* scheduler task ID */
};

/* Singleton driver object */
static struct swtimer swtimer;

/* -------------------------------------------------------------------------- */

static irqreturn_t swtimer_isr(int irq, void *data)
{
	struct swtimer *obj = (struct swtimer *)data;

	UNUSED(irq);

	/*
	 * For some reason, sometimes we catch the interrupts with CC1IF..CC4IF
	 * flags set in TIM2_SR register (though we didn't enable such
	 * interrupts in TIM2_DIER). It can be some errata, but anyway, let's
	 * increment ticks only on "Update" interrupt flag.
	 */
	if (!timer_get_flag(obj->hw_tim.base, TIM_SR_UIF))
		return IRQ_NONE;

	/*
	 * TODO: Implement this one:
	 *   - increment global ticks counter
	 *   - set task state to "Ready" via scheduler
	 *   - clear HW timer flag
	 *   - account for possible race conditions (if any)
	 */
}

static void swtimer_task(void *data)
{
	struct swtimer *obj = (struct swtimer *)data;

	/*
	 * TODO: Implement this one:
	 *   - for each active SW timer:
	 *     - decrement remaining time for each tick of global tick counter
	 *     - if remaining time is <= 0:
	 *       - call timer callback
	 *       - set remaining time to period time
	 *   - zero the global "ticks" counter
	 *
	 */
}

/* -------------------------------------------------------------------------- */

/**
 * Initialize software timer framework.
 *
 * Setup underneath hardware timer and run it.
 *
 * @param[in] hw_tim Parameters of HW timer to use
 * @return 0 on success or negative number on error
 *
 * @note Scheduler must be initialized before calling this
 */
int swtimer_init(const struct swtimer_hw_tim *hw_tim)
{
	/*
	 * TODO: Implement this one:
	 *   - copy info from "hw_tim" param to driver object
	 *   - setup IRQ (request IRQ, enable IRQ in NVIC + set priority)
	 *   - setup and enable hardware timer
	 *   - add scheduler task for handling SW timers
	 */
}

/**
 * De-initialize software timer framework.
 */
void swtimer_exit(void)
{
	/*
	 * TODO: Implement this one:
	 *   - disable hardware timer
	 *   - unregister the IRQ
	 *   - remove scheduler task
	 *   - clean driver object
	 */
}

/**
 * Reset internal tick counter.
 *
 * This function can be useful e.g. in case when swtimer_init() was called
 * early, and when system initialization is complete, the timer tick counter is
 * non-zero value, but it's unwanted to call sw timer callbacks yet.
 */
void swtimer_reset(void)
{
	/*
	 * TODO: Implement this one:
	 *   - set global ticks counter to 0
	 */
}

/**
 * Register software timer and start it immediately.
 *
 * @param cb Timer callback; will be executed when timer is expired
 * @param period Timer period, msec; minimal period (and granularity): 5 msec
 * @return Timer ID (handle) starting from 1, or negative value on error
 *
 * @note This function can be used before swtimer_init()
 */
int swtimer_tim_register(swtimer_callback_t cb, int period)
{
	int slot;

	cm3_assert(cb != NULL);
	cm3_assert(period >= SWTIMER_HW_OVERFLOW);

	/*
	 * TODO: Implement this one:
	 *   - find empty (not used) slot in timer table
	 *   - configure all fields for timer with found slot using
	 *     this function params (timer must start when this function ends)
	 *   - return registered timer ID number (not slot number)
	 */
}

/**
 * Delete specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_del(int id)
{
	/*
	 * TODO: Implement this one:
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - clear all fields in timer table for timer with specified ID
	 */
}

/**
 * Reset and start specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_start(int id)
{
	/*
	 * TODO: Implement this one:
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - reset timer by ID
	 *   - make timer with specified slot number "active"
	 */
}

/**
 * Stop specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_stop(int id)
{
	/*
	 * TODO: Implement this one:
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - make timer with specified slot number "not active"
	 */
}

/**
 * Reset software timer by ID.
 *
 * @param id Timer handle
 */
void swtimer_tim_reset(int id)
{
	/*
	 * TODO: Implement this one:
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - set remaining time to timer period, for timer with specified slot
	 *     number
	 */
}

/**
 * Set new pediod for timer by ID.
 *
 * @param id Timer handle
 * @param period Timer period, msec
 */
void swtimer_tim_set_period(int id, int period)
{
	/*
	 * TODO: Implement this one:
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - set period for timer with specified slot number
	 */
}

/**
 * Get remaining time till the timer expires.
 *
 * @param id Timer handle
 * @return Remaining time, msec
 */
int swtimer_tim_get_remaining(int id)
{
	/*
	 * TODO: Implement this one:
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - return remaining time for timer with specified slot number
	 */
}
