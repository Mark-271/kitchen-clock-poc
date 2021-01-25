#include <ds18b20.h>
#include <common.h>
#include <tools.h>
#include <libopencm3/stm32/gpio.h>
#include <stddef.h>

#define TEMPERATURE_CONV_TIME 900

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

/**
 * Read temperature register from DS18B20.
 *
 * @param obj 1-wire device object
 * @return Parsed value
 */
struct tempval ds18b20_get_temperature(struct ow *obj)
{
	unsigned long flags;
	struct tempval tv;
	int8_t data[2];
	size_t i;

	ow_reset_pulse(obj);
	ow_write_byte(obj, SKIP_ROM);
	ow_write_byte(obj, CONVERT_T);
	enter_critical(flags);
	mdelay(TEMPERATURE_CONV_TIME);
	exit_critical(flags);
	ow_reset_pulse(obj);
	ow_write_byte(obj, SKIP_ROM);
	ow_write_byte(obj, READ_SCRATCHPAD);

	for (i = 0; i < 2; i++)
		data[i] = ow_read_byte(obj);
	ow_reset_pulse(obj);

	tv = ds18b20_parse_temp(data[0], data[1]);
	return tv;
}

/**
 * Convert temperature data to null-terminated string.
 *
 * @param tv Contains parsed temperature register from DS18B20
 * @param str Array to store string literal
 * @return Pointer to @ref str
 */
char *tempval_to_str(struct tempval *tv, char str[])
{
	int i = 0;
	uint16_t rem;

	if (!tv->frac) {
		str[i++] = '0';
	} else {
		while (tv->frac) {
			rem = tv->frac % 10;
			str[i++] = rem + '0';
			tv->frac /= 10;
		}
	}
	str[i++] = '.';
	if (!tv->integer) {
		str[i++] = '0';
	} else {
		while (tv->integer) {
			rem = tv->integer % 10;
			str[i++] = rem + '0';
			tv->integer /= 10;
		}
	}
	str[i++] = tv->sign;
	str[i] = '\0';
	inplace_reverse(str);

	return str;
}
