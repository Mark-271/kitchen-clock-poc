#include <keyboard.h>
#include <common.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>

/**
 * Keypad polling.
 *
 * Alternately polls every button looking for pushed one.
 *
 * @param obj Keypad object must be initialized before call
 * @return Code (1..4) of pushed button or 0 on failure
 */
static int kbd_poll(struct kbd *obj)
{
	int code = 0;

	gpio_set(obj->port, obj->r2);
	if (!gpio_get(obj->port, obj->l1))
		code =  1;
	if (!gpio_get(obj->port, obj->l2))
		code =  2;
	gpio_clear(obj->port, obj->r1 | obj->r2);

	gpio_set(obj->port, obj->r1);
	if (!gpio_get(obj->port, obj->l1))
		code = 3;
	if (!gpio_get(obj->port, obj->l2))
		code = 4;
	gpio_clear(obj->port, obj->r1 | obj->r2);

	return code;
}

static void kbd_exti_init(void)
{
	rcc_periph_clock_enable(RCC_AFIO);

	nvic_enable_irq(NVIC_EXTI1_IRQ);
	nvic_enable_irq(NVIC_EXTI2_IRQ);

	exti_select_source(EXTI1, GPIOA);
	exti_select_source(EXTI2, GPIOA);
	exti_set_trigger(EXTI1, EXTI_TRIGGER_FALLING);
	exti_set_trigger(EXTI2, EXTI_TRIGGER_FALLING);
	exti_enable_request(EXTI1);
	exti_enable_request(EXTI2);
}

static void kbd_timer_init(void)
{
	rcc_periph_clock_enable(RCC_TIM4);

	timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP );
	timer_set_prescaler(TIM4, 999999); /* Timer frequency is 1 MHz */
	timer_set_period(TIM4, 9999); /* Overflow occures every 10 ms */
	timer_one_shot_mode(TIM4);

	nvic_enable_irq(NVIC_TIM4_IRQ);
	timer_enable_irq(TIM4, TIM_DIER_UIE);
}

int kbd_init(struct kbd *obj, kbd_btn_event_t cb)
{
	/* Sampling lines should be configured with pull up resistor */
	gpio_set(obj->port, obj->l1 | obj->l2);
	/* Scan lines should be in low state */
	gpio_clear(obj->port, obj->r1 | obj->r2);

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
	timer_clear_flag(TIM4, TIM_SR_UIF);
}
