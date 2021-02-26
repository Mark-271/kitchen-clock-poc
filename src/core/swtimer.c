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
#include <core/irq.h>
#include <core/sched.h>
#include <tools/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

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
	struct irq_action action;
	struct swtimer_sw_tim timer_list[SWTIMER_TIMERS_MAX];
	volatile int ticks;		/* global ticks counter */
	int task_id;			/* scheduler task ID */
};

/* Singleton driver object */
static struct swtimer swtimer;

/* -------------------------------------------------------------------------- */
static irqreturn_t swtimer_isr(int irq, void *data)
{
	struct swtimer *obj = (struct swtimer *)(data);

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
	 *   - increment global ticks counter
	 *   - set task state to "Ready" via scheduler
	 *   - clear HW timer flag
	 *   - account for possible race conditions
	 */
	swtimer.ticks += SWTIMER_HW_OVERFLOW;
	sched_set_ready(obj->task_id);
	timer_clear_flag(obj->hw_tim.base, TIM_SR_UIF);

	return IRQ_HANDLED;
}

static void swtimer_task(void *data)
{
	size_t i;
	struct swtimer *obj = (struct swtimer *)data;

	/*
	 *   - for each active SW timer:
	 *     - decrement remaining time for each tick of global tick counter
	 *     - if remaining time is <= 0:
	 *       - call timer callback
	 *       - set remaining time to period time
	 *   - zero the global "ticks" counter
	 */
	for (i = 0; i < SWTIMER_TIMERS_MAX; i++) {
		if (obj->timer_list[i].active) {
			obj->timer_list[i].remaining -= obj->ticks;
			if (obj->timer_list[i].remaining <= 0) {
				obj->timer_list[i].cb();
				obj->timer_list[i].remaining = obj->timer_list[i].period;
			}
		}
	}
	obj->ticks = 0;
}
/* -------------------------------------------------------------------------- */

static int swtimer_find_empty_slot(void)
{
	size_t i;

	for (i = 0; i < SWTIMER_TIMERS_MAX; ++i) {
		if (!swtimer.timer_list[i].cb)
			return i;
	}

	return -1;
}

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
	 *   - copy info from "hw_tim" param to driver object
	 *   - setup IRQ (request IRQ, enable IRQ in NVIC + set priority)
	 *   - setup and enable hardware timer
	 *   - add scheduler task for handling SW timers
	 */
	int ret;
	swtimer.hw_tim = *hw_tim;
	swtimer.action.handler = swtimer_isr;
	swtimer.action.irq = swtimer.hw_tim.irq;
	swtimer.action.name = SWTIMER_TASK;
	swtimer.action.data = &swtimer;

	ret = irq_request(&swtimer.action);
	if (ret < 0) {
		printf("Can't register interrupt handler for timer\n");
		return ret;
	}

	rcc_periph_reset_pulse(swtimer.hw_tim.rst);
	timer_set_mode(swtimer.hw_tim.base, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(swtimer.hw_tim.base, swtimer.hw_tim.psc);
	timer_set_period(swtimer.hw_tim.base, swtimer.hw_tim.arr);
	timer_disable_preload(swtimer.hw_tim.base);
	timer_continuous_mode(swtimer.hw_tim.base);
	timer_enable_update_event(swtimer.hw_tim.base);
	timer_update_on_overflow(swtimer.hw_tim.base);

	nvic_enable_irq(swtimer.hw_tim.irq);
	nvic_set_priority(swtimer.hw_tim.irq, 1);
	timer_enable_irq(swtimer.hw_tim.base, TIM_DIER_UIE);
	timer_enable_counter(swtimer.hw_tim.base);

	ret = sched_add_task(SWTIMER_TASK, swtimer_task, &swtimer,
			     &swtimer.task_id);
	if (ret < 0) {
		printf("Can't add task\n");
		return ret;
	}
	return 0;
}

/**
 * De-initialize software timer framework.
 */
void swtimer_exit(void)
{
	/*
	 *   - disable hardware timer
	 *   - unregister the IRQ
	 *   - remove scheduler task
	 *   - clean driver object
	 */
	timer_disable_counter(swtimer.hw_tim.base);
	timer_disable_irq(swtimer.hw_tim.base, TIM_DIER_UIE);
	nvic_disable_irq(swtimer.hw_tim.irq);
	sched_del_task(swtimer.task_id);
	irq_free(&swtimer.action);
	UNUSED(swtimer);
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
	/* Set global ticks counter to 0 */
	swtimer.ticks = 0;
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
	/*
	 *   - find empty (not used) slot in timer table
	 *   - configure all fields for timer with found slot using
	 *     this function params (timer must start when this function ends)
	 *   - return registered timer ID number (not slot number)
	 */
	int slot;

	cm3_assert(cb != NULL);
	cm3_assert(period >= SWTIMER_HW_OVERFLOW);

	slot = swtimer_find_empty_slot();
	if (slot < 0)
		return -1;

	swtimer.timer_list[slot].cb = cb;
	swtimer.timer_list[slot].period = period;
	swtimer.timer_list[slot].remaining = period;
	swtimer.timer_list[slot].active = true;

	return slot + 1;
}

/**
 * Delete specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_del(int id)
{
	int slot = id - 1;

	cm3_assert(slot >= 0 && slot < SWTIMER_TIMERS_MAX);
	memset(&swtimer.timer_list[slot], 0, sizeof(struct swtimer_sw_tim));
}

/**
 * Reset and start specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_start(int id)
{
	/*
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - reset timer by ID
	 *   - make timer with specified slot number "active"
	 */
	int slot = id -1;

	cm3_assert(slot >= 0 && slot < SWTIMER_TIMERS_MAX);
	swtimer_tim_reset(id);
	swtimer.timer_list[slot].active = true;
}

/**
 * Stop specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_stop(int id)
{
	/*
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - make timer with specified slot number "not active"
	 */
	int slot = id -1;

	cm3_assert(slot >= 0 && slot < SWTIMER_TIMERS_MAX);
	swtimer_tim_reset(id);
	swtimer.timer_list[slot].active = false;
}

/**
 * Reset software timer by ID.
 *
 * @param id Timer handle
 */
void swtimer_tim_reset(int id)
{
	/*
	 *   - calculate slot number from id
	 *   - add assert() to check if slot number is not in range
	 *   - set remaining time to timer period, for timer with specified slot
	 *     number
	 */
	int slot = id -1;

	cm3_assert(slot >= 0 && slot < SWTIMER_TIMERS_MAX);
	swtimer.timer_list[slot].remaining = swtimer.timer_list[slot].period;
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
