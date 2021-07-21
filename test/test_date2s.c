#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#define TM_START_YEAR		1900
#define MIN_TM_YEAR		0
#define MIN_REGS_YEAR		0
#define MAX_REGS_YEAR		99
#define TM_MDAYS		30


struct test_data {
	struct tm tm;
	char *date;
};

static struct test_data test_data[] = {
	{
		.tm = {
			.tm_sec = 0, .tm_min = 0, .tm_hour = 0,
			.tm_mday = 1, .tm_mon = 0, .tm_year = 121,
			.tm_wday = 0, .tm_yday = 142, .tm_isdst = 0
		},
		.date = "SUN 01JAN2021",
	},
	{
		.tm = {
			.tm_sec = 59, .tm_min = 59, .tm_hour = 23,
			.tm_sec = 0, .tm_min = 0, .tm_hour = 0,
			.tm_mday = 31, .tm_mon = 11, .tm_year = 121,
			.tm_wday = 6, .tm_yday = 364, .tm_isdst = 0
		},
		.date = "SAT 31DEC2021",
	},
	{
		.tm = {
			.tm_sec = 0, .tm_min = 6, .tm_hour = 22,
			.tm_mday = 9, .tm_mon = 3, .tm_year = 121,
			.tm_wday = 5, .tm_yday = 98, .tm_isdst = 0
		},
		.date = "FRI 09APR2021",
	},
};

/* ------------------------------------------------------------------------- */

static inline void int4_to_str(char *s, int n)
{
	s[0] = n / 1000 + '0';
	s[1] = (n / 100) % 10 + '0';
	s[2] = (n / 10) % 10 + '0';
	s[3] = n % 10 + '0';
}

static inline void int2_to_str(char *s, int n)
{
	s[0] = n / 10 + '0';
	s[1] = n % 10 + '0';
}

/**
 * Convert date to string.
 *
 * @param tm Contains date values, i.e., day, month, year, week day
 * @param s Buffer to store string of the form, e.g. "MON 19JUL2021"
 */
void date2str(struct tm *tm, char *s)
{
	const int year = (tm->tm_year + TM_START_YEAR);
	static const char *wdays[7] = {
		"SUN",
		"MON",
		"TUE",
		"WED",
		"THU",
		"FRI",
		"SAT"
	};
	static const char *months[12] = {
		"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
		"JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
	};

	strcpy(s, wdays[tm->tm_wday]);
	int2_to_str(s + 4, tm->tm_mday);
	s[3] = ' ';
	strcpy(s + 6, months[tm->tm_mon]);
	int4_to_str(s + 9, year);
	s[13] = '\0';
}

/* -------------------------------------------------------------------------- */

static bool test_date2str(void)
{
	size_t i;
	char buf[25];

	printf("%s\n", __func__);

	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		date2str(&test_data[i].tm, buf);

		if (strcmp(buf, test_data[i].date) != 0)
			goto err;
	}

	printf("[SUCCESS]\n");

	return true;

err:
	printf("[FAIL]\n");
	fprintf(stderr, "Error inside %lu entry\n", i);
	printf("buf->%s\ndate->%s\n", buf, test_data[i].date);

	return false;
}

int main(void)
{
	bool res;

	res = test_date2str();
	if (!res)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
