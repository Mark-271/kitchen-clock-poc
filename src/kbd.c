#include <kbd.h>
#include <common.h>
#include <irq.h>
#include <sched.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
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
/* Set number of irqs */
#define KBD_IRQS			3

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

static void kbd_exti_init(struct kbd *obj)
{
	size_t i;

	for (i = 0; i < KBD_READ_LINES; i++) {
		nvic_enable_irq(obj->gpio.irq[i]);
		exti_select_source(obj->gpio.read[i], obj->gpio.port);
		exti_set_trigger(obj->gpio.read[i], obj->gpio.trigger);
		exti_enable_request(obj->gpio.read[i]);
	}
}

static void kbd_disable_exti(struct kbd *obj)
{
	size_t i;

	for (i = 0; i < KBD_READ_LINES; i++) {
		exti_disable_request(obj->gpio.read[i]);
		nvic_disable_irq(obj->gpio.irq[i]);
	};
}

static void kbd_enable_exti(struct kbd *obj)
{
	size_t i;

	for (i = 0; i < KBD_READ_LINES; i++) {
		nvic_enable_irq(obj->gpio.irq[i]);
		exti_enable_request(obj->gpio.read[i]);
	};
}

static void kdb_handle_interrupt(struct kbd *obj)
{
	if (!obj->scan_pending) {
		timer_enable_counter(TIM4);
		kbd_disable_exti(obj);
		obj->scan_pending = true;
	}
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

	obj->scan_pending = false;
	kbd_enable_exti(obj);

	/* Issue callback for each changed button state */
	for (i = 0; i < KEYS; ++i) {
		if (pressed_now[i] && !obj->pressed[i]) {
			obj->cb(i, true);
			obj->pressed[i] = true;
			ret = 0;
		} else if (!pressed_now[i] && obj->pressed[i]) {
			obj->cb(i, false);
			obj->pressed[i] = false;
			ret = 0;
		}
	}

	if (ret == -1)
		printf("Warning: No button is pressed\n");
}

static irqreturn_t exti1_handler(int irq, void *data)
{
	struct kbd *obj = (struct kbd *)(data);

	UNUSED(irq);

	kdb_handle_interrupt(obj);
	exti_reset_request(EXTI1);

	return IRQ_HANDLED;
}

static irqreturn_t exti2_handler(int irq, void *data)
{
	struct kbd *obj = (struct kbd *)(data);

	UNUSED(irq);

	kdb_handle_interrupt(obj);
	exti_reset_request(EXTI2);

	return IRQ_HANDLED;
}

static irqreturn_t tim4_handler(int irq, void *data)
{
	struct kbd *obj = (struct kbd *)(data);

	UNUSED(irq);

	if (!timer_get_flag(TIM4, TIM_SR_UIF))
		return IRQ_NONE;

	sched_set_ready(obj->btn_task_id);
	timer_clear_flag(TIM4, TIM_SR_UIF);

	return IRQ_HANDLED;
}

/* Store irq objects */
static struct irq_action a[KBD_IRQS] = {
	{
		.handler = exti1_handler,
		.irq = NVIC_EXTI1_IRQ,
		.name = "kbd_scan1",
	},
	{
		.handler = exti2_handler,
		.irq = NVIC_EXTI2_IRQ,
		.name = "kbd_scan2",
	},
	{
		.handler = tim4_handler,
		.irq = NVIC_TIM4_IRQ,
		.name = "kbd_timer",
	}
};

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

	ret = sched_add_task("keyboard", kbd_task, obj, &obj->btn_task_id);
	if (ret < 0)
		return -1;

	kbd_exti_init(obj);
	kbd_timer_init();

	/* Register interrupt handlers */
	for (i = 0; i < KBD_IRQS; i++) {
		a[i].data = (void *)obj;
		ret = irq_request(&a[i]);
		if (ret < 0)
			return ret;
	}

	/* Scan lines should be in low state */
	gpio_clear(obj->gpio.port, obj->scan_mask);

	return 0;
}

void kbd_exit(struct kbd *obj)
{
	size_t i;

	timer_disable_irq(TIM4, TIM_DIER_UIE);
	nvic_disable_irq(NVIC_TIM4_IRQ);
	kbd_disable_exti(obj);

	/* Remove interrupt handlers */
	for (i = 0; i < KBD_IRQS; i++)
		irq_free(&a[i]);

	UNUSED(obj);
}
