#include <drivers/kbd.h>
#include <core/irq.h>
#include <core/swtimer.h>
#include <tools/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <stddef.h>
#include <stdio.h>

/* Calculate button number by current scan line and read line numbers */
#define BTN_LOOKUP(i, j)	((j) + (i) * KBD_READ_LINES)
/* Set period for timer in msec */
#define KBD_TIM_PERIOD		10
/* Set total number of keyboard buttons */
#define KEYS			4
/* Set number of irqs */
#define KBD_IRQS		2

static int kbd_gpio2irq(uint16_t gpio)
{
	if (gpio == GPIO0)
		return NVIC_EXTI0_IRQ;
	else if (gpio == GPIO1)
		return NVIC_EXTI1_IRQ;
	else if (gpio == GPIO2)
		return NVIC_EXTI2_IRQ;
	else if (gpio == GPIO3)
		return NVIC_EXTI3_IRQ;
	else if (gpio == GPIO4)
		return NVIC_EXTI4_IRQ;
	else if (gpio >= GPIO5 && gpio <= GPIO9)
		return NVIC_EXTI9_5_IRQ;
	else if (gpio >= GPIO10 && gpio <= GPIO15)
		return NVIC_EXTI15_10_IRQ;
	else
		return -1;
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
		swtimer_tim_start(obj->timer_id);
		kbd_disable_exti(obj);
		obj->scan_pending = true;
	}
}

/*
 * Find out pressed button(s) and pass it to callback.
 */
static void kbd_handle_btn(void *data)
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
		udelay(CONFIG_GPIO_STAB_DELAY);

		/* Read all read lines */
		val = gpio_port_read(obj->gpio.port) & obj->read_mask;
		for (j = 0; j < KBD_READ_LINES; ++j) {
			btn = BTN_LOOKUP(i, j);
			pressed_now[btn] = !(val & obj->gpio.read[j]);
		}
	}

	gpio_clear(obj->gpio.port, obj->scan_mask);
	udelay(CONFIG_GPIO_STAB_DELAY);

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
	exti_reset_request(obj->gpio.irq[0]);

	return IRQ_HANDLED;
}

static irqreturn_t exti2_handler(int irq, void *data)
{
	struct kbd *obj = (struct kbd *)(data);

	UNUSED(irq);

	kdb_handle_interrupt(obj);
	exti_reset_request(obj->gpio.irq[1]);

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
	}
};

/**
 * Initialize the keyboard driver.
 *
 * @param obj Driver's objects
 * @param[in] gpio GPIO port and lines where the keyboard is connected
 * @param cb Callback to call when some button is pressed
 * @return 0 or negative value on error
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

	/* Prepare irq values for external interrupts */
	for (i = 0; i < KBD_READ_LINES; i++) {
		ret = kbd_gpio2irq(gpio->read[i]);
		if (ret < 0)
			return -1;
		obj->gpio.irq[i] = ret;
	}

	kbd_exti_init(obj);

	obj->timer_id = swtimer_tim_register(kbd_handle_btn, obj,
					     KBD_TIM_PERIOD);
	if (obj->timer_id < 0)
		return -1;
	swtimer_tim_stop(obj->timer_id); /* timer should be triggered by exti */

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

	swtimer_tim_del(obj->timer_id);
	kbd_disable_exti(obj);

	/* Remove interrupt handlers */
	for (i = 0; i < KBD_IRQS; i++)
		irq_free(&a[i]);

	UNUSED(obj);
}
