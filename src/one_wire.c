#include <stdint.h>
#include <libopencm3/stm32/gpio.h>
#include <one_wire.h>
#include <delay.h>

void write_bit(uint32_t gpio_port, uint16_t gpio_pin, uint8_t bit)
{
	gpio_clear(gpio_port, gpio_pin);
	udelay(bit ? WRITE_1_TIME : WRITE_0_TIME);

	gpio_set(gpio_port, gpio_pin);
	if (bit)
		udelay(WRITE_1_PAUSE);
}

uint16_t read_bit(uint32_t gpio_port, uint16_t gpio_pin)
{
	uint16_t bit = 0;

	gpio_clear(gpio_port, gpio_pin);
	udelay(READ_INIT_TIME);
	gpio_set(gpio_port, gpio_pin);
	udelay(READ_SAMPLING_TIME);

	bit = gpio_get(gpio_port, gpio_pin);
	udelay(READ_PAUSE);

	return ((bit != 0) ? 1 : 0);
	}
