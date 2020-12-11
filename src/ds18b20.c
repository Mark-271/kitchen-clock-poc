#include <libopencm3/stm32/gpio.h>
#include <ds18b20.h>
#include <delay.h>

/* Public functions */
void ow_init(struct ow *ow, uint32_t gpio_port, uint16_t gpio_pin)
{
	ow->port = gpio_port;
	ow->pin = gpio_pin;
}

void ow_exit(struct ow *ow)
{
}
