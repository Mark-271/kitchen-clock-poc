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
#include <core/wdt.h>
#include <tools/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <stddef.h>
#include <string.h>

#define SWTIMER_TASK		"swtimer"

/* Driver struct (swtimer framework) */
struct swtimer {
	struct swtimer_hw_tim hw_tim;
	struct irq_action action;
	int ticks;			/* global ticks counter */
	int task_id;			/* scheduler task ID */
	int wdt_tid;			/* watchdog timer task ID */
	int max_slot;			/* max timer slot */
	struct swtimer_sw_tim *timer;	/* list of timers */
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

	WRITE_ONCE(obj->ticks, SWTIMER_HW_OVERFLOW);
	sched_set_ready(obj->task_id);
	timer_clear_flag(obj->hw_tim.base, TIM_SR_UIF);

	return IRQ_HANDLED;
}

static void swtimer_task(void *data)
{
	struct swtimer_sw_tim *tim;

	UNUSED(data);

	for (tim = swtimer.timer; tim; tim = tim->next) {
		if (!tim->active)
			continue;
		if (tim->remaining <= 0) {
			tim->cb(tim->data);
			tim->remaining = tim->period;
		}
		tim->remaining -= READ_ONCE(swtimer.ticks);
	}
	WRITE_ONCE(swtimer.ticks, 0);

	wdt_task_report(swtimer.wdt_tid);
}

static void swtimer_hw_init(struct swtimer *obj)
{
	rcc_periph_reset_pulse(obj->hw_tim.rst);

	timer_set_mode(obj->hw_tim.base, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(obj->hw_tim.base, obj->hw_tim.psc);
	timer_set_period(obj->hw_tim.base, obj->hw_tim.arr);
	timer_disable_preload(obj->hw_tim.base);
	timer_continuous_mode(obj->hw_tim.base);
	timer_enable_update_event(obj->hw_tim.base);
	timer_update_on_overflow(obj->hw_tim.base);
	timer_enable_irq(obj->hw_tim.base, TIM_DIER_UIE);

	nvic_set_priority(obj->hw_tim.irq, 1);
	nvic_enable_irq(obj->hw_tim.irq);

	timer_enable_counter(obj->hw_tim.base);
}

static struct swtimer_sw_tim *swtimer_find_tim(struct swtimer_sw_tim *t, int i)
{
	if (t == NULL)
		return NULL;

	if (t->id == i)
		return t;

	return swtimer_find_tim(t->next, i);
}

/* -------------------------------------------------------------------------- */

/**
 * Reset internal tick counter.
 *
 * This function can be useful e.g. in case when swtimer_init() was called
 * early, and when system initialization is complete, the timer tick counter is
 * non-zero value, but it's unwanted to call sw timer callbacks yet.
 */
void swtimer_reset(void)
{
	WRITE_ONCE(swtimer.ticks, 0); /* Set global ticks counter to 0 */
}

/**
 * Register software timer and start it immediately.
 *
 * @param tim Linked list containing software timers
 * @return Timer ID (handle) starting from 1
 *
 * @note This function can be used before swtimer_init()
 */
int swtimer_tim_register(struct swtimer_sw_tim *tim)
{
	struct swtimer_sw_tim *t;

	cm3_assert(tim->cb != NULL);
	cm3_assert(tim->period >= SWTIMER_HW_OVERFLOW);

	tim->id = swtimer.max_slot + 1;
	tim->remaining = tim->period;
	tim->active = true;
	tim->next = NULL;

	if (swtimer.timer) {
		t = swtimer.timer;
		while (t->next)
			t = t->next;
		t->next = tim;
	} else {
		swtimer.timer = tim;
	}

	return swtimer.max_slot++;
}

/**
 * Delete specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_del(int id)
{
	struct swtimer_sw_tim *tim;

	tim = swtimer_find_tim(swtimer.timer, id);
	if (tim == NULL)
		return;

	if (swtimer.timer == tim) {
		swtimer.timer = NULL;
	} else {
		struct swtimer_sw_tim *t;

		for (t = swtimer.timer; t; t = t->next) {
			if (t->next == tim) {
				t->next = t->next->next;
				tim->next = NULL;
			}
		}
	}
}

/**
 * Reset and start specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_start(int id)
{
	struct swtimer_sw_tim *tim;

	tim = swtimer_find_tim(swtimer.timer, id);
	if (tim == NULL)
		return;

	tim->active = true;
}

/**
 * Stop specified timer.
 *
 * @param id Timer handle
 */
void swtimer_tim_stop(int id)
{
	struct swtimer_sw_tim *tim;

	tim = swtimer_find_tim(swtimer.timer, id);
	if (tim == NULL)
		return;

	tim->active = false;
}

/**
 * Reset software timer by ID.
 *
 * @param id Timer handle
 */
void swtimer_tim_reset(int id)
{
	struct swtimer_sw_tim *tim;

	tim = swtimer_find_tim(swtimer.timer, id);
	if (tim == NULL)
		return;

	tim->remaining = tim->period;
}

/**
 * Set new pediod for timer by ID.
 *
 * @param id Timer handle
 * @param period Timer period, msec
 */
void swtimer_tim_set_period(int id, int period)
{
	struct swtimer_sw_tim *tim;

	tim = swtimer_find_tim(swtimer.timer, id);
	if (tim == NULL)
		return;

	tim->period = period;
}

/**
 * Get remaining time till the timer expires.
 *
 * @param id Timer handle
 * @return Remaining time, msec
 */
int swtimer_tim_get_remaining(int id)
{
	struct swtimer_sw_tim *tim;

	tim = swtimer_find_tim(swtimer.timer, id);
	if (tim == NULL)
		return -1;

	return tim->remaining;
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
	int ret;
	struct swtimer *obj = &swtimer;

	obj->hw_tim		= *hw_tim;
	obj->action.handler	= swtimer_isr;
	obj->action.irq		= hw_tim->irq;
	obj->action.name	= SWTIMER_TASK;
	obj->action.data	= obj;

	ret = irq_request(&obj->action);
	if (ret < 0)
		return -1;

	swtimer_hw_init(obj);

	ret = sched_add_task(SWTIMER_TASK, swtimer_task, obj, &obj->task_id);
	if (ret < 0)
		return -2;

	swtimer.wdt_tid = wdt_task_add(SWTIMER_TASK);
	if (swtimer.wdt_tid < 0)
		return -3;

	return 0;
}

/**
 * De-initialize software timer framework.
 */
void swtimer_exit(void)
{
	timer_disable_counter(swtimer.hw_tim.base);
	timer_disable_irq(swtimer.hw_tim.base, TIM_DIER_UIE);
	nvic_disable_irq(swtimer.hw_tim.irq);
	wdt_task_del(swtimer.wdt_tid);
	sched_del_task(swtimer.task_id);
	irq_free(&swtimer.action);
	UNUSED(swtimer);
}
