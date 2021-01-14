#include <stdint.h>

#include <libopencm3/cm3/cortex.h>
#include <libopencm3/stm32/gpio.h>

#include <common.h>
#include <delay.h>
#include <ds18b20.h>
#include <one_wire.h>

/**
 * Parse temperature register from DS18B20.
 *
 * @param lsb Least significant byte of temperature register
 * @param msb Most significant byte of temperature register
 * @return Parsed value
 */
static struct tempval ds18b20_parse_temp(uint8_t lsb, uint8_t msb)
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

struct tempval ds18b20_get_temperature(struct ow *ow)
{
	struct tempval temp;
	int8_t data[2];
	int i;

	cm_disable_interrupts();
	ow_reset_pulse(ow);
	ow_write_byte(ow, SKIP_ROM);
	ow_write_byte(ow, CONVERT_T);

	cm_enable_interrupts();
	mdelay(900);
	cm_disable_interrupts();

	ow_reset_pulse(ow);
	ow_write_byte(ow, SKIP_ROM);
	ow_write_byte(ow, READ_SCRATCHPAD);
	cm_enable_interrupts();

	for (i = 0; i < 2; i++)
		data[i] = ow_read_byte(ow);
	cm_disable_interrupts();
	ow_reset_pulse(ow);
	cm_enable_interrupts();

	temp = ds18b20_parse_temp(data[0], data[1]);
	return temp;
}
