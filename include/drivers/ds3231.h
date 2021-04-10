#ifndef DRIVERS_DS3231_H
#define DRIVERS_DS3231_H

#include <libopencm3/stm32/exti.h>
#include <stdint.h>

typedef void (*ds3231_callback_t)(void);

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

/* RTC hardware parameters */
struct ds3231_device {
	uint32_t port;
	uint16_t pin;
	int irq;			/* exti number */
	enum exti_trigger_type trig; /* exti trigger condition */
	uint32_t i2c_base;
	uint8_t addr;
};

/* RTC API */
int ds3231_init(const struct ds3231_device *obj, int epoch_year);
void ds3231_exit(const struct ds3231_device *obj);
int ds3231_read_time(struct rtc_time *tm);
int ds3231_set_time(struct rtc_time *tm);

#endif /* DRIVERS_DS3231_H */
