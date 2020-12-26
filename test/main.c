#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BIT(n)		(1 << n)
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

struct tempval {
	uint16_t integer:12;
	uint16_t frac;
	char sign;		/* '-' or '+' */
};

struct test_data {
	struct tempval temp;	/* parsed temp */
	uint8_t lsb;		/* LSB register data */
	uint8_t msb;		/* MSB register data */
};

static struct test_data test_data[] = {
	{ { 125, 0, '+' },
	  .msb = 0b00000111, .lsb = 0b11010000 },
	{ { 85, 0, '+' },
	  .msb = 0b00000101, .lsb = 0b01010000 },
	{ { 25, 625, '+' },
	  .msb = 0b00000001, .lsb = 0b10010001 },
	{ { 10, 1250, '+' },
	  .msb = 0b00000000, .lsb = 0b10100010 },
	{ { 0, 5000, '+' },
	  .msb = 0b00000000, .lsb = 0b00001000 },
	{ { 0, 0, '+' },
	  .msb = 0b00000000, .lsb = 0b00000000 },
	{ { 0, 5000, '-' },
	  .msb = 0b11111111, .lsb = 0b11111000 },
	{ { 10, 1250, '-' },
	  .msb = 0b11111111, .lsb = 0b01011110 },
	{ { 25, 625, '-' },
	  .msb = 0b11111110, .lsb = 0b01101111 },
	{ { 55, 0, '-' },
	  .msb = 0b11111100, .lsb = 0b10010000 },
};

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

static bool test_parse_temp(void)
{
	int i;
	struct tempval temp;
	struct test_data d;

	printf("TESTING %s()...\n", __func__);
	for (i = 0; i < ARRAY_SIZE(test_data); ++i) {
		d = test_data[i];
		temp = parse_temp(d.lsb, d.msb);
		if (temp.integer != d.temp.integer || temp.frac != d.temp.frac
		    || temp.sign != d.temp.sign) {
			goto err;
		}
	}

	printf("[SUCCESS]\n");
	return true;

err:
	printf("[FAIL]\n");
	fprintf(stderr, "Test data: %c%d.%d\n", d.temp.sign, d.temp.integer,
		d.temp.frac);
	fprintf(stderr, "Parsed value: %c%d.%d\n", temp.sign, temp.integer,
		temp.frac);
	return false;
}

int main(void)
{
	bool res;

	res = test_parse_temp();
	if (!res)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
