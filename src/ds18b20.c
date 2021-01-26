#include <ds18b20.h>
#include <one_wire.h>
#include <common.h>
#include <tools.h>
#include <libopencm3/stm32/gpio.h>
#include <stddef.h>

#define TEMPERATURE_CONV_TIME		900

#define CMD_SKIP_ROM			0xcc
#define CMD_CONVERT_T			0x44
#define CMD_READ_SCRATCHPAD		0xbe

static struct ow ow;

/**
 * Parse temperature register from DS18B20.
 *
 * @param lsb Least significant byte of temperature register
 * @param msb Most significant byte of temperature register
 * @return Parsed value
 */
static struct ds18b20_temp ds18b20_parse_temp(uint8_t lsb, uint8_t msb)
{
	struct ds18b20_temp tv;

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

int ds18b20_init(struct ds18b20 *obj)
{
	int ret;

	ow.port = obj->port;
	ow.pin = obj->pin;

	ret = ow_init(&ow);

	return ret;
}

/* Destroy ds18b20 object */
void ds18b20_exit(struct ds18b20 *obj)
{
	UNUSED(obj);
	ow_exit(&ow);
}

/**
 * Read temperature register from DS18B20.
 *
 * @param obj 1-wire device object
 * @return Parsed value
 */
struct ds18b20_temp ds18b20_read_temp(struct ds18b20 *obj)
{
	unsigned long flags;
	int8_t data[2];
	size_t i;

	ow_reset_pulse(&ow);
	ow_write_byte(&ow, CMD_SKIP_ROM);
	ow_write_byte(&ow, CMD_CONVERT_T);
	enter_critical(flags);
	mdelay(TEMPERATURE_CONV_TIME);
	exit_critical(flags);
	ow_reset_pulse(&ow);
	ow_write_byte(&ow, CMD_SKIP_ROM);
	ow_write_byte(&ow, CMD_READ_SCRATCHPAD);

	for (i = 0; i < 2; i++)
		data[i] = ow_read_byte(&ow);
	ow_reset_pulse(&ow);

	obj->temp = ds18b20_parse_temp(data[0], data[1]);
	return obj->temp;
}

/**
 * Convert temperature data to null-terminated string.
 *
 * @param obj Contains parsed temperature register from DS18B20
 * @param str Array to store string literal
 * @return Pointer to @ref str
 */
char *ds18b20_temp2str(struct ds18b20_temp *obj, char str[])
{
	int i = 0;
	uint16_t rem;

	if (!obj->frac) {
		str[i++] = '0';
	} else {
		while (obj->frac) {
			rem = obj->frac % 10;
			str[i++] = rem + '0';
			obj->frac /= 10;
		}
	}
	str[i++] = '.';
	if (!obj->integer) {
		str[i++] = '0';
	} else {
		while (obj->integer) {
			rem = obj->integer % 10;
			str[i++] = rem + '0';
			obj->integer /= 10;
		}
	}
	str[i++] = obj->sign;
	str[i] = '\0';
	inplace_reverse(str);

	return str;
}
