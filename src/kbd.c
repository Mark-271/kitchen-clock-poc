#include <kbd.h>
#include <common.h>
#include <sched.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>
#include <stddef.h>
#include <stdio.h>

/* Calculate button number by current scan line and read line numbers */
#define BTN_LOOKUP(i, j)		((j) + (i) * KBD_READ_LINES)
#define SCAN_LINE_DELAY			10 /* usec */
/* Set timer prescaler value to obtain frequency 1 MHz */
#define TIM_PRESCALER			((rcc_ahb_frequency) / 1e6)
/* Set counter period to trigger overflow every 10 msec */
#define TIM_PERIOD			1e4
/* Set total number of keyboard buttons */
#define KEYS				4

static int btn_task_id;
static bool scan_pending;	/* Allows external interrupts */
static bool pressed[KEYS];	/* Store state of every button */


static void disable_exti(void)
{
	exti_disable_request(EXTI1);
	exti_disable_request(EXTI2);
	nvic_disable_irq(NVIC_EXTI1_IRQ);
	nvic_disable_irq(NVIC_EXTI2_IRQ);
}

static void enable_exti(void)
{
	nvic_enable_irq(NVIC_EXTI1_IRQ);
	nvic_enable_irq(NVIC_EXTI2_IRQ);
	exti_enable_request(EXTI1);
	exti_enable_request(EXTI2);
}

/*
 * Task function for scheduler.
 * Find out pressed button(s) and pas it to callback.
 */
static void kbd_task(void *data)
{
	struct kbd *obj = (struct kbd *)(data);
	int btn;
	size_t i;
	uint16_t val;
	int ret = -1;
	bool pressed_now[KEYS];

	/* Find out the state of each button */
	for (i = 0; i < KBD_SCAN_LINES; ++i) {
		size_t j;

		/* Scan next line */
		gpio_set(obj->gpio.port, obj->scan_mask);
		gpio_clear(obj->gpio.port, obj->gpio.scan[i]);
		udelay(SCAN_LINE_DELAY); /* Wait for voltage to stabilize */

		/* Read all read lines */
		val = gpio_port_read(obj->gpio.port) & obj->read_mask;
		for (j = 0; j < KBD_READ_LINES; ++j) {
			btn = BTN_LOOKUP(i, j);
			pressed_now[btn] = !(val & obj->gpio.read[j]);
		}
	}

	gpio_clear(obj->gpio.port, obj->scan_mask);
	udelay(SCAN_LINE_DELAY); /* Wait for voltage to stabilize */
	scan_pending = false;

	/* Issue callback for each changed button state */
	for (i = 0; i < KEYS; ++i) {
		if (pressed_now[i] && !pressed[i]) {
			obj->cb(i, true);
			pressed[i] = true;
			ret = 0;
		} else if (!pressed_now[i] && pressed[i]) {
			obj->cb(i, false);
			pressed[i] = false;
			ret = 0;
		}
	}

	if (ret == -1)
		printf("Warning: No button is pressed\n");
}

static void kbd_exti_init(void)
{
	size_t i;

	rcc_periph_clock_enable(RCC_AFIO);

	for (i = 0; i < ARRAY_SIZE(exti); i++) {
		nvic_enable_irq(exti[i].irq);
		exti_select_source(exti[i].line, exti[i].port);
		exti_set_trigger(exti[i].line, exti[i].trigger);
		exti_enable_request(exti[i].line);
	}
}

static void kbd_timer_init(void)
{
	rcc_periph_clock_enable(RCC_TIM4);

	timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
	timer_set_prescaler(TIM4, TIM_PRESCALER);
	timer_set_period(TIM4, TIM_PERIOD);
	timer_one_shot_mode(TIM4);

	nvic_enable_irq(NVIC_TIM4_IRQ);
	timer_enable_irq(TIM4, TIM_DIER_UIE);
}

/**
 * Initialize the keyboard driver.
 *
 * @param obj Driver's objects
 * @param[in] gpio GPIO port and lines where the keyboard is connected
 * @param cb Callback to call when some button is pressed
 *
 * @note Read lines should be configured with pull up resistor before
 *       running this function.
 */
int kbd_init(struct kbd *obj, const struct kbd_gpio *gpio, kbd_btn_event_t cb)
{
	int ret;
	size_t i;

	cm3_assert(obj != NULL);
	cm3_assert(gpio != NULL);
	cm3_assert(cb != NULL);

	obj->gpio = *gpio;

	/* Prepare mask values for scan and read pins */
	obj->scan_mask = obj->read_mask = 0;
	for (i = 0; i < KBD_SCAN_LINES; ++i)
		obj->scan_mask |= gpio->scan[i];
	for (i = 0; i < KBD_READ_LINES; ++i)
		obj->read_mask |= gpio->read[i];

	obj->cb = cb; /* register the callback */

	ret = sched_add_task("keyboard", kbd_task, obj, &btn_task_id);
	if (ret < 0)
		return -1;

	/* Scan lines should be in low state */
	gpio_clear(obj->gpio.port, obj->scan_mask);

	kbd_exti_init();
	kbd_timer_init();

	return 0;
}

void kbd_exit(struct kbd *obj)
{
	timer_disable_irq(TIM4, TIM_DIER_UIE);
	nvic_disable_irq(NVIC_TIM4_IRQ);

	exti_disable_request(EXTI1);
	exti_disable_request(EXTI2);
	nvic_disable_irq(NVIC_EXTI1_IRQ);
	nvic_disable_irq(NVIC_EXTI2_IRQ);

	UNUSED(obj);
}

void exti1_isr(void)
{
	timer_enable_counter(TIM4);
	exti_reset_request(EXTI1);
}

void exti2_isr(void)
{
	timer_enable_counter(TIM4);
	exti_reset_request(EXTI2);
}

void tim4_isr(void)
{
	if (!timer_get_flag(TIM4, TIM_SR_UIF))
		return;

	sched_set_ready(btn_task_id);

	timer_clear_flag(TIM4, TIM_SR_UIF);
}
