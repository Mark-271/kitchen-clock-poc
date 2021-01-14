#include <stdint.h>

#include <libopencm3/stm32/gpio.h>

#include <common.h>
#include <delay.h>
#include <one_wire.h>

static unsigned long flags;

static void ow_write_bit(struct ow *ow, uint8_t bit)
{
	gpio_clear(ow->port, ow->pin);
	enter_critical(flags);
	udelay(bit ? WRITE_1_TIME : WRITE_0_TIME);
	exit_critical(flags);

	gpio_set(ow->port, ow->pin);
	if (bit) {
		enter_critical(flags);
		udelay(WRITE_1_PAUSE);
		exit_critical(flags);
	}
}

static uint16_t ow_read_bit(struct ow *ow)
{
	uint16_t bit = 0;

	gpio_clear(ow->port, ow->pin);
	enter_critical(flags);
	udelay(READ_INIT_TIME);
	exit_critical(flags);

	gpio_set(ow->port, ow->pin);
	enter_critical(flags);
	udelay(READ_SAMPLING_TIME);
	bit = gpio_get(ow->port, ow->pin);
	udelay(READ_PAUSE);
	exit_critical(flags);

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
int ow_init(struct ow *ow)
{
	gpio_set(ow->port, ow->pin);
	/* if device holds bus in low lvl return error */
	enter_critical(flags);
	if (!(gpio_get(ow->port, ow->pin))) {
		exit_critical(flags);
		return -1;
	}
	exit_critical(flags);

	return ow_reset_pulse(ow);
}

void ow_exit(struct ow *ow)
{
	gpio_clear(ow->port, ow->pin);
	UNUSED(ow);
}

int ow_reset_pulse(struct ow *ow)
{
	int ret;

	gpio_clear(ow->port, ow->pin);
	enter_critical(flags);
	udelay(RESET_TIME);
	exit_critical(flags);

	gpio_set(ow->port, ow->pin);
	enter_critical(flags);
	udelay(PRESENCE_WAIT_TIME);
	ret = gpio_get(ow->port, ow->pin);
	exit_critical(flags);
	if (ret)
		return -1;

	enter_critical(flags);
	udelay(RESET_TIME);
	exit_critical(flags);

	enter_critical(flags);
	ret = gpio_get(ow->port, ow->pin);
	exit_critical(flags);
	if (!ret)
		return -2;

	return 0;
}

void ow_write_byte(struct ow *ow, uint8_t byte)
{
	int i;

	for (i = 0; i < 8; i++) {
		ow_write_bit(ow, byte >> i & 1);
		enter_critical(flags);
		udelay(SLOT_WINDOW);
		exit_critical(flags);
	}
}

int8_t ow_read_byte(struct ow *ow)
{
	int16_t byte = 0;
	int i;

	for (i = 0; i < 8; i++) {
		byte |= ow_read_bit(ow) << i;
		enter_critical(flags);
		udelay(SLOT_WINDOW);
		exit_critical(flags);
	}

	return (int8_t)byte;
}
