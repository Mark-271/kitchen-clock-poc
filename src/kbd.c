#include <kbd.h>
#include <common.h>
#include <sched.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>
#include <stddef.h>

/* Calculate button number by current scan line and read line numbers */
#define BTN_LOOKUP(i, j)		((j) + (i) * KBD_READ_LINES)
#define SCAN_LINE_DELAY			10 /* usec */
/* Set timer prescaler value to obtain frequency 1 MHz */
#define TIM_PRESCALER			((rcc_ahb_frequency) / 1e6)
/* Set counter period to trigger overflow every 10 msec */
#define TIM_PERIOD			1e4
/* Set tota number of keyboard buttons */
#define KEYS				4

static int btn_task_id;
static bool pressed[KEYS];

/**
 * Read pressed button.
 *
 * Alternatively sets scanning gpio lines to "high" state and scans
 * reading gpio lines looking for "low" state (i.e. button pressed)
 * on one of them. Reads register allocated for keyboard.
 *
 * @param obj Keyboard object
 * @return Button code or -1 if no pressed button found
 */
static int kbd_read_btn(struct kbd *obj)
{
	uint16_t val;
	size_t i;

	for (i = 0; i < KBD_SCAN_LINES; ++i) {
		size_t j;

		/* Scan next line */
		gpio_set(obj->gpio.port, obj->scan_mask);
		gpio_clear(obj->gpio.port, obj->gpio.scan[i]);
		udelay(SCAN_LINE_DELAY); /* Wait for voltage to stabilize */

		/* Read all read lines */
		val = gpio_port_read(obj->gpio.port) & obj->read_mask;
		for (j = 0; j < KBD_READ_LINES; ++j) {
			if (!(val & obj->gpio.read[j])) {
				pressed[BTN_LOOKUP(i, j)] = true;
				return BTN_LOOKUP(i, j);
			}
			if (pressed[BTN_LOOKUP(i, j)]) {
				pressed[BTN_LOOKUP(i, j)] = false;
				return BTN_LOOKUP(i, j);
			}
		}
	}

	return -1;
}

/*
 * Task function for scheduler.
 * Define pressed button and pass it to callback.
 */
static void kbd_task(void *data)
{
	struct kbd *obj = (struct kbd *)(data);
	int btn;

	UNUSED(obj);

	btn = kbd_read_btn(obj);
	gpio_clear(obj->gpio.port, obj->scan_mask);

	obj->cb(btn, pressed[btn]);
}

static void kbd_exti_init(void)
{
	rcc_periph_clock_enable(RCC_AFIO);

	nvic_enable_irq(NVIC_EXTI1_IRQ);
	nvic_enable_irq(NVIC_EXTI2_IRQ);

	exti_select_source(EXTI1, GPIOA);
	exti_select_source(EXTI2, GPIOA);
	exti_set_trigger(EXTI1, EXTI_TRIGGER_BOTH);
	exti_set_trigger(EXTI2, EXTI_TRIGGER_BOTH);
	exti_enable_request(EXTI1);
	exti_enable_request(EXTI2);
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
