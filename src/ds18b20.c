#include <stdint.h>
#include <libopencm3/stm32/gpio.h>
#include <ds18b20.h>
#include <delay.h>

/* Static functions */
static void write_bit(uint32_t gpio_port, uint16_t gpio_pin, uint8_t bit)
{
	gpio_clear(gpio_port, gpio_pin);
	udelay(bit ? WRITE_1_TIME : WRITE_0_TIME);
	gpio_set(gpio_port, gpio_pin);
	udelay(bit ? WRITE_1_PAUSE : WRITE_0_PAUSE);
}

/* Public functions */
void ow_init(struct ow *ow, uint32_t gpio_port, uint16_t gpio_pin)
{
	ow->port = gpio_port;
	ow->pin = gpio_pin;

	gpio_set(ow->port, ow->pin);
}

void ow_exit(struct ow *ow)
{
	UNUSED(ow);
}

int ow_reset(struct ow *ow)
{
	int ret;

	gpio_clear(ow->port, ow->pin);
	udelay(RESET_TIME);
	gpio_set(ow->port, ow->pin);
	udelay(PRESENCE_WAIT_TIME);
	ret = gpio_get(ow->port, ow->pin);
	udelay(RESET_TIME - PRESENCE_WAIT_TIME);

	return ret;
}

void ow_write_byte(struct ow *ow, uint8_t byte)
{
	int i;

	for (i = 0; i < 8; i++) {
		write_bit(ow->port, ow->pin, byte >> i & 1);
		udelay(SLOT_WINDOW);
	}
}
