// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

#include <tools/tools.h>
#include <string.h>

#define TM_START_YEAR		1900

static inline void int2_to_str(char *s, int n)
{
	s[0] = n / 10 + '0';
	s[1] = n % 10 + '0';
}

static inline void int4_to_str(char *s, int n)
{
	s[0] = n / 1000 + '0';
	s[1] = (n / 100) % 10 + '0';
	s[2] = (n / 10) % 10 + '0';
	s[3] = n % 10 + '0';
}

static int yisleap(int year)
{
	return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/**
 * Calculate day number beginning from January 1.
 *
 * @param mon Month should be in the range 0 - 11
 * @param day Month day in range 1 - 31
 * @param year Number representing the year
 * @return Day number in the range 1 - 366
 */
int get_yday(int mon, int day, int year)
{
	static const int days[2][13] = {
		{0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
		{0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335}
	};
	int leap = yisleap(year);

	return days[leap][mon] + day;
}

/**
 *  Reverse the given null-terminated string in place.
 *
 *  Swap the values in the two given variables.
 *  Fails when a and b refer to same memory location.
 *  This works because of three basic properties of XOR:
 *  x ^ 0 = x, x ^ x = 0 and x ^ y = y ^ x for all values x and y
 *
 *  @param str should be an array, whose contents are initialized to
 *  the given string constant.
 */
void inplace_reverse(char *str)
{
	if (str) {
		char *end = str + strlen(str) - 1;

#define XOR_SWAP(a, b) do	\
		{		\
			a ^= b;	\
			b ^= a;	\
			a ^= b;	\
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
 * Convert time data to string.
 *
 * @param tm Contains time values, i.e., hours, minutes and  seconds
 * @param s Buffer to store string of the form "HH:MM"
 */
void time2str(struct tm *tm, char *s)
{
	int2_to_str(s, tm->tm_hour);
	int2_to_str(s + 3, tm->tm_min);
	s[2] = ':';
	s[5] = '\0';
}

/**
 * Convert date to string.
 *
 * @param tm Contains date values, i.e., day, month, year, week day
 * @param s Buffer to store string of the form, e.g. "MON 19 JUL 2021"
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
	s[6] = ' ';
	strcpy(s + 7, months[tm->tm_mon]);
	int4_to_str(s + 11, year);
	s[10] = ' ';
	s[15] = '\0';
}
