#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#define N 10

struct test_data {
	uint8_t bin;
	uint8_t bcd;
};

static struct test_data test_data[] = {
	{ 0, 0 },
	{ 1, 1 },
	{ 9, 9 },
	{ 10, 16 },
	{ 11, 17 },
	{ 19, 25 },
	{ 20, 32 },
	{ 29, 41 },
	{ 90, 144 },
	{ 99, 153 },
};

/**
 * Convert from binary to binary coded decimal (bcd) format.
 *
 * Implemented only for decimal two-digit numbers
 *
 * @param val Initial binary number
 * @return Encoded value
 */
static uint8_t dec2bcd(uint8_t val)
{
	return ((val / 10) << 4) | (val % 10);
}

/**
 * Convert from bcd to binary.
 *
 * @param val Encoded bcd value
 * @return Decoded binary number
 */
static uint8_t bcd2dec(uint8_t val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

static bool test_bcd(void)
{
	size_t i;
	int val;

	for (i = 0; i < N; i++) {
		val = dec2bcd(test_data[i].bin);
		if (val != test_data[i].bcd)
			goto err_bcd;
	}

	for (i = 0; i < N; i++) {
		val = bcd2dec(test_data[i].bcd);
		if (val != test_data[i].bin)
			goto err_bin;
	}

	printf("[SUCCESS]\n");
	return true;

err_bcd:
	printf("[FAIL]\n");
	fprintf(stderr, "Bcd data: %d\n", test_data[i].bcd);
	return false;

err_bin:
	printf("[FAIL]\n");
	fprintf(stderr, "Bin data: %d\n", test_data[i].bin);
	return false;
}

int main(void)
{
	bool res;

	res = test_bcd();
	if (!res)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
