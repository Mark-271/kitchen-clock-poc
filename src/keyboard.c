#include <keyboard.h>
#include <common.h>
#include <libopencm3/stm32/gpio.h>

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
