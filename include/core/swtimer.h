/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef CORE_SWTIMER_H
#define CORE_SWTIMER_H

#include <libopencm3/stm32/rcc.h>
#include <stdint.h>

/* HW timer granularity (min SW timer period), msec */
#define SWTIMER_HW_OVERFLOW	5

typedef void (*swtimer_callback_t)(void *data);

/* Hardware timer parameters */
struct swtimer_hw_tim {
	uint32_t base;			/* base register addr; e.g. TIM2 */
	uint8_t irq;			/* IRQ number; e.g. NVIC_TIM2_IRQ */
	enum rcc_periph_rst rst;	/* reset offset, e.g. RST_TIM2 */

	uint32_t arr;			/* period value, for 5 msec overflow */
	uint32_t psc;			/* prescaler val, for 5 msec overflow */
};

/* Software timer parameters */
struct swtimer_sw_tim {
	swtimer_callback_t cb;	/* function to call when this timer overflows */
	void *data;		/* user private data passed to cb */
	int period;		/* timer overflow period, msec */
	int remaining;		/* remaining time till overflow, msec */
	bool active;		/* if true, callback will be executed */
	int id;			/* timer ID */
	struct swtimer_sw_tim *next;
};

/* Global swtimer fwk API */
int swtimer_init(const struct swtimer_hw_tim *hw_tim);
void swtimer_exit(void);
void swtimer_reset(void);

/* SW timers API */
int swtimer_tim_register(struct swtimer_sw_tim *tim);
void swtimer_tim_del(int id);
void swtimer_tim_start(int id);
void swtimer_tim_stop(int id);
void swtimer_tim_reset(int id);
void swtimer_tim_set_period(int id, int period);
int swtimer_tim_get_remaining(int id);

#endif /* CORE_SWTIMER_H */
