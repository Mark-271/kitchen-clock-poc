#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE(a)	(sizeof(a) / sizeof(a[0]))

#define TM_START_YEAR		1900
#define MIN_TM_YEAR		0
#define MIN_REGS_YEAR		0
#define MAX_REGS_YEAR		99
#define TM_MDAYS		30

struct rtc_time {
	int tm_sec;	/* seconds after the minute	0-59	*/
	int tm_min;	/* minutes after the hour	0-59	*/
	int tm_hour;	/* hours since midnight		0-23	*/
	int tm_mday;	/* day of the month		1-31	*/
	int tm_mon;	/* months since January		0-11	*/
	int tm_year;	/* years since 1900			*/
	int tm_wday;	/* days since Sunday		0-6	*/
	int tm_yday;	/* days since January 1		0-365	*/
	int tm_isdst;	/* Daylight Saving Time flag		*/
};

struct ds3231_regs {
	uint8_t ss;	/* 0-59 */
	uint8_t mm;	/* 0-59 */
	uint8_t hh;	/* 0-23 */
	uint8_t day;	/* 1-7 */
	uint8_t date;	/* 1-31 */
	uint8_t month;	/* 1-12 + century */
	uint8_t year;	/* 0-99 */
};

struct test_data {
	struct rtc_time tm;
	struct ds3231_regs regs;
};

static struct test_data test_data[] = {
	{
		.tm = {
			.tm_sec = 0, .tm_min = 0, .tm_hour = 0, .tm_mday = 1,
			.tm_mon = 0, .tm_year = 121, .tm_wday = 0, .tm_yday = 0,
			.tm_isdst = 0
		},
		.regs = {
			.ss = 0, .mm = 0, .hh = 0, .day = 1, .date = 1,
			.month = 1, .year = 0
		},
	},
	{
		.tm = {
			.tm_sec = 59, .tm_min = 59, .tm_hour = 23,
			.tm_mday = 31, .tm_mon = 11, .tm_year = 121,
			.tm_wday = 6, .tm_yday = 364, .tm_isdst = 0
		},
		.regs = {
			.ss = 59, .mm = 59, .hh = 23, .day = 7, .date = 31,
			.month = 12, .year = 0
		},
	},
	{
		.tm = {
			.tm_sec = 0, .tm_min = 6, .tm_hour = 22,
			.tm_mday = 9, .tm_mon = 3, .tm_year = 121,
			.tm_wday = 5, .tm_yday = 98, .tm_isdst = 0
		},
		.regs = {
			.ss = 0, .mm = 6, .hh = 22, .day = 6, .date = 9,
			.month = 4, .year = 0
		},
	},
	{
		.tm = {
			.tm_sec = 0, .tm_min = 6, .tm_hour = 22,
			.tm_mday = 31, .tm_mon = 11, .tm_year = 124,
			.tm_wday = 5, .tm_yday = 365, .tm_isdst = 0
		},
		.regs = {
			.ss = 0, .mm = 6, .hh = 22, .day = 6, .date = 31,
			.month = 12, .year = 3
		},
	},
};

static int epoch_year = 2021;

/* ------------------------------------------------------------------------- */

static int yisleap(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

static int get_yday(int mon, int day, int year)
{
	static const int days[2][12] = {
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
	};
	int leap = yisleap(year);

	return days[leap][mon] + day;
}

static bool ds3231_time2regs(const struct rtc_time *tm,
			     struct ds3231_regs *regs)
{
	const int regs_year = tm->tm_year + TM_START_YEAR - epoch_year;

	if (regs_year > MAX_REGS_YEAR || regs_year < MIN_REGS_YEAR)
		return false;

	regs->ss	= tm->tm_sec;
	regs->mm	= tm->tm_min;
	regs->hh	= tm->tm_hour;
	regs->day	= tm->tm_wday + 1;
	regs->date	= tm->tm_mday;
	regs->month	= tm->tm_mon + 1;
	regs->year	= regs_year;

	return true;
}

static bool ds3231_regs2time(const struct ds3231_regs *regs,
			     struct rtc_time *tm)
{
	const int year = regs->year + epoch_year;
	const int tm_year = year - TM_START_YEAR;

	if (tm_year < MIN_TM_YEAR)
		return false;

	tm->tm_sec	= regs->ss;
	tm->tm_min	= regs->mm;
	tm->tm_hour	= regs->hh;
	tm->tm_mday	= regs->date;
	tm->tm_mon	= regs->month - 1;
	tm->tm_year	= tm_year;
	tm->tm_wday	= regs->day - 1;
	tm->tm_yday	= get_yday(tm->tm_mon, tm->tm_mday, year) - 1;
	tm->tm_isdst	= 0;

	return true;
}

/* -------------------------------------------------------------------------- */

bool test_time2regs(void)
{
	size_t i;
	struct ds3231_regs temp = {0};

	printf("%s\n", __func__);

	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		ds3231_time2regs(&test_data[i].tm, &temp);

		if (temp.ss != test_data[i].regs.ss) {
			printf("ss = %d\n", temp.ss);
			goto err;
		} else if (temp.mm != test_data[i].regs.mm) {
			printf("mm = %d\n", temp.mm);
			goto err;
		} else if (temp.hh !=test_data[i].regs.hh) {
			printf("hh = %d\n", temp.hh);
			goto err;
		} else if (temp.day != test_data[i].regs.day) {
			printf("day = %d\n", temp.day);
			goto err;
		} else if (temp.date != test_data[i].regs.date) {
			printf("date = %d\n", temp.date);
			goto err;
		} else if (temp.month != test_data[i].regs.month) {
			printf("month = %d\n", temp.month);
			goto err;
		} else if (temp.year != test_data[i].regs.year) {
			printf("year = %d\n", temp.year);
			goto err;
		}
	}

	printf("[SUCCESS]\n");

	return true;

err:
	printf("[FAIL]\n");
	fprintf(stderr, "Error inside %lu entry\n", i);
	return false;
}

bool test_regs2time(void)
{
	size_t i;
	struct rtc_time temp;

	printf("%s\n", __func__);

	for (i = 0; i < ARRAY_SIZE(test_data); i++) {
		ds3231_regs2time(&test_data[i].regs, &temp);

		if (temp.tm_sec != test_data[i].tm.tm_sec) {
			printf("%d\n", temp.tm_sec);
			goto err;
		} else if (temp.tm_min != test_data[i].tm.tm_min) {
			printf("%d\n", temp.tm_min);
			goto err;
		} else if (temp.tm_hour !=test_data[i].tm.tm_hour) {
			printf("%d\n", temp.tm_hour);
			goto err;
		} else if (temp.tm_mday != test_data[i].tm.tm_mday) {
			printf("%d\n", temp.tm_mday);
			goto err;
		} else if (temp.tm_mon != test_data[i].tm.tm_mon) {
			printf("%d\n", temp.tm_mon);
			goto err;
		} else if (temp.tm_year != test_data[i].tm.tm_year) {
			printf("%d\n", temp.tm_year);
			goto err;
		} else if (temp.tm_wday != test_data[i].tm.tm_wday) {
			printf("%d\n", temp.tm_wday);
			goto err;
		} else if (temp.tm_yday != test_data[i].tm.tm_yday) {
			printf("%d\n", temp.tm_yday);
			goto err;
		} else if (temp.tm_isdst != test_data[i].tm.tm_isdst) {
			printf("%d\n", temp.tm_isdst);
			goto err;
		}
	}

	printf("[SUCCESS]\n");

	return true;

err:
	printf("[FAIL]\n");
	fprintf(stderr, "Error inside %lu entry\n", i);
	return false;
}

int main(void)
{
	bool res;

	res = test_time2regs();
	if (!res)
		return EXIT_FAILURE;

	res = test_regs2time();
	if (!res)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}
