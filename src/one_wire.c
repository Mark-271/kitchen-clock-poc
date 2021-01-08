#include <stdint.h>

#include <libopencm3/stm32/gpio.h>

#include <common.h>
#include <delay.h>
#include <one_wire.h>

static void ow_write_bit(struct ow *ow, uint8_t bit)
{
	gpio_clear(ow->port, ow->pin);
	udelay(bit ? WRITE_1_TIME : WRITE_0_TIME);

	gpio_set(ow->port, ow->pin);
	if (bit)
		udelay(WRITE_1_PAUSE);
}

static uint16_t ow_read_bit(struct ow *ow)
{
	uint16_t bit = 0;

	gpio_clear(ow->port, ow->pin);
	udelay(READ_INIT_TIME);
	gpio_set(ow->port, ow->pin);
	udelay(READ_SAMPLING_TIME);

	bit = gpio_get(ow->port, ow->pin);
	udelay(READ_PAUSE);

	return ((bit != 0) ? 1 : 0);
}

/**
 * Initialize one-wire interface.
 *
 * @param ow Structure to store corresponding GPIOs
 * @param gpio_port Unsigned 32int. GPIO port identifier
 * @param gpio_pin Unsigned 16int. GPIO pin identifier
 * @return 0 when success and -1 otherwise
 */
int ow_init(struct ow *ow, uint32_t gpio_port, uint16_t gpio_pin)
{
	ow->port = gpio_port;
	ow->pin = gpio_pin;

	gpio_set(ow->port, ow->pin);
	/* if device holds bus in low lvl return error */
	if (!(gpio_get(ow->port, ow->pin)))
		return -1;

	return 0;
}

void ow_exit(struct ow *ow)
{
	gpio_clear(ow->port, ow->pin);
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
	if (ret)
		return -1;

	udelay(RESET_TIME - PRESENCE_WAIT_TIME);

	return ret;
}

void ow_write_byte(struct ow *ow, uint8_t byte)
{
	int i;

	for (i = 0; i < 8; i++) {
		ow_write_bit(ow, byte >> i & 1);
		udelay(SLOT_WINDOW);
	}
}

int8_t ow_read_byte(struct ow *ow)
{
	int16_t byte = 0;
	int i;

	for (i = 0; i < 8; i++) {
		byte |= ow_read_bit(ow) << i;
		udelay(SLOT_WINDOW);
	}

	return (int8_t)byte;
}
