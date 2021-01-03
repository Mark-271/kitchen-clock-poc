#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BIT(n)		(1 << n)
#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

struct tempval {
	uint16_t integer:12;
	uint16_t frac;
	char sign;		/* '-' or '+' */
};

struct test_data {
	struct tempval temp;	/* parsed temp */
	char *temp_str;		/* stringisized temp */
};

static struct test_data test_data[] = {
	{ { 125, 0, '+' },
	  .temp_str = "+125.0" },
	{ { 85, 0, '+' },
	  .temp_str = "+85.0" },
	{ { 25, 625, '+' },
	  .temp_str = "+25.625" },
	{ { 10, 1250, '+' },
	  .temp_str = "+10.1250" },
	{ { 0, 5000, '+' },
	  .temp_str = "+0.5000" },
	{ { 0, 0, '+' },
	  .temp_str = "+0.0" },
	{ { 0, 5000, '-' },
	  .temp_str = "-0.5000" },
	{ { 10, 1250, '-' },
	  .temp_str = "-10.1250" },
	{ { 25, 625, '-' },
	  .temp_str = "-25.625" },
	{ { 55, 0, '-' },
	  .temp_str = "-55.0" },
};

/**
 *  Reverse the given null-terminated string in place.
 *
 *  Swap the values in the two given variables
 *  Fails when a and b refer to same memory location
 *  This works because of three basic properties of xor:
 *  x ^ 0 = x, x ^ x = 0 and x ^ y = y ^ x for all values x and y
 *
 *  @param input should be an array, whose contents are initialized to
 *  the given string constant.
 */
static void inplace_reverse(char *str)
{
	if (str) {
		char *end = str + strlen(str) - 1;

#define XOR_SWAP(a,b) do\
		{\
			a ^= b;\
			b ^= a;\
			a ^= b;\
		} while (0)

		/* Walk inwards from both ends of the string,
		 * swapping until we get to the middle
		 */
		while (str < end) {
			XOR_SWAP(*str, *end);
			str++;
			end--;
		}
#undef XOR_SWAP
	}
}

/**
 * Convert temperature data to null-terminated string.
 *
 * @param tv Contains parsed temperature register from DS18B20 @ref parse_temp()
 * @param str Array to store string literal
 * @return Pointer to string
 *
 */
static char* tempval_to_str(struct tempval *tv, char str[])
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

static bool test_tempval_to_str(void)
{
	int i;
	struct test_data t;
	char buf[10];

	printf("TESTING %s()...\n", __func__);
	for (i = 0; i < ARRAY_SIZE(test_data); ++i) {
		t = test_data[i];
		tempval_to_str(&t.temp, buf);
		if ((strcmp(buf, t.temp_str)) != 0)
			goto err;
	}

	printf("[SUCCESS]\n");
	return true;

err:
	printf("[FAIL]\n");
	fprintf(stderr, "Test data: %s\n", t.temp_str);
	fprintf(stderr, "Parsed value: %s\n", buf);
	return false;
}

int main(void)
{
	bool res;

	res = test_tempval_to_str();
	if (!res)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
