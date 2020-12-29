#include <stdint.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/cortex.h>
#include <common.h>
#include <one_wire.h>
#include <ds18b20.h>
#include <delay.h>

/* Static functions */
static int ow_reset(struct ow *ow)
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

static void ow_write_byte(struct ow *ow, uint8_t byte)
{
	int i;

	for (i = 0; i < 8; i++) {
		write_bit(ow->port, ow->pin, byte >> i & 1);
		udelay(SLOT_WINDOW);
	}
}

static int8_t ow_read_byte(struct ow *ow)
{
	int16_t byte = 0;
	int i;

	for (i = 0; i < 8; i++) {
		byte |= read_bit(ow->port, ow->pin) << i;
		udelay(SLOT_WINDOW);
	}

	return (int8_t)byte;
}

/**
 * Parse temperature register from DS18B20.
 *
 * @param lsb Least significant byte of temperature register
 * @param msb Most significant byte of temperature register
 * @return Parsed value
 */
static struct tempval parse_temp(uint8_t lsb, uint8_t msb)
{
	struct tempval tv;

	tv.integer = (msb << 4) | (lsb >> 4);
	if (msb & BIT(7)) {
		tv.sign = '-';
		/*
		 * Handle negative 2's complement frac value:
		 *   1. Take first 4 bits from LSB (negative part)
		 *   2. Append missing 1111 bits (0xf0), to be able to invert
		 *      8-bit variable
		 *   3. -1 and invert (to handle 2's complement format),
		 *      accounting for implicit integer promotion rule (by
		 *      casting result to uint8_t)
		 */
		tv.frac = 625 * (uint8_t)(~(((lsb & 0xf) - 1) | 0xf0));
		/* Handle negative 2's complement integer value */
		tv.integer = ~tv.integer;
		if (tv.frac == 0)
			tv.integer++;
	} else {
		tv.sign = '+';
		tv.frac = 625 * (lsb & 0xf);
	}

	return tv;
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
	gpio_clear(ow->port, ow->pin);
	UNUSED(ow);
}

struct tempval get_temperature(struct ow *ow)
{
	struct tempval temp;
	int8_t data[2];
	int i;

	cm_disable_interrupts();
	ow_reset(ow);
	ow_write_byte(ow, SKIP_ROM);
	ow_write_byte(ow, CONVERT_T);

	cm_enable_interrupts();
	mdelay(900);
	cm_disable_interrupts();

	ow_reset(ow);
	ow_write_byte(ow, SKIP_ROM);
	ow_write_byte(ow, READ_SCRATCHPAD);
	cm_enable_interrupts();

	for (i = 0; i < 2; i++)
		data[i] = ow_read_byte(ow);
	cm_disable_interrupts();
	ow_reset(ow);
	cm_enable_interrupts();

	temp = parse_temp(data[0], data[1]);
	return temp;
}
