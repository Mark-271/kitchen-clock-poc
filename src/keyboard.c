#include <keyboard.h>
#include <common.h>
#include <libopencm3/stm32/gpio.h>

/**
 * Keyboard polling.
 *
 * Alternately polls every button looking for pushed one.
 *
 * @param obj Keyboard object must be initialized before call
 * @return Code (1..4) of pushed button or 0 on failure
 */
static int keyboard_poll(struct kb *obj)
{
	uint16_t ret;

	gpio_set(obj->port, obj->r1_pin);
	if (!(ret = gpio_get(obj->port, obj->l1_pin))) {
		gpio_clear(obj->port, obj->r1_pin | obj->r2_pin);
		return 1;
	} else if (!(ret = gpio_get(obj->port, obj->l2_pin))) {
		gpio_clear(obj->port, obj->r1_pin | obj->r2_pin);
		return 2;
	}
	gpio_toggle(obj->port, obj->r1_pin | obj->r2_pin);
	if (!(ret = gpio_get(obj->port, obj->l1_pin))) {
		gpio_clear(obj->port, obj->r1_pin | obj->r2_pin);
		return 3;
	} else if (!(ret = gpio_get(obj->port, obj->l2_pin))) {
		gpio_clear(obj->port, obj->r1_pin | obj->r2_pin);
		return 4;
	}

	gpio_clear(obj->port, obj->r1_pin | obj->r2_pin);
	return 0;
}

int keyboard_init(struct kb *obj)
{
	/* Sampling lines should be configured with pull up resistor */
	gpio_set(obj->port, obj->l1_pin | obj->l2_pin);
	/* Scan lines should be in low state */
	gpio_clear(obj->port, obj->r1_pin | obj->r2_pin);

	return 0;
}

void keyboard_exit(struct kb *obj)
{
	UNUSED(obj);
}

/* Returns code name of pushed button on keyboard */
int keyboard_push_button(struct kb *obj)
{
	return keyboard_poll(obj);
}
