#ifndef RTC_H
#define RTC_H

#include <stdint.h>

struct tm {
	uint8_t ss;
	uint8_t mm;
	uint8_t hh;
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;
};

struct rtc {
	uint32_t i2c_base;
	uint8_t addr;
	struct tm tm;
};

int rtc_init(struct rtc *obj, uint32_t base, uint8_t addr);
void rtc_exit(struct rtc *obj);
void rtc_get_date(struct rtc *obj);

#endif /* RTC_H */
