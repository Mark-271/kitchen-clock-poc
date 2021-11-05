/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef DRIVERS_DS3231_H
#define DRIVERS_DS3231_H

#include <core/irq.h>
#include <libopencm3/stm32/exti.h>
#include <stdint.h>

#define TM_START_YEAR		1900

typedef void (*ds3231_alarm_callback_t)(void);

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

/* RTC hardware parameters */
struct ds3231_device {
	uint32_t port;
	uint16_t pin;
	int irq;			/* exti number */
	enum exti_trigger_type trig; /* exti trigger condition */
	uint32_t i2c_base;
	uint8_t addr;
};

struct ds3231_alarm {
	int task_id;
	bool status;
	struct irq_action action;
	struct rtc_time time;
	ds3231_alarm_callback_t cb;
};

/* Driver structure */
struct ds3231 {
	int epoch_year;
	struct ds3231_alarm alarm;
	struct ds3231_device device;
	struct ds3231_regs regs;
};

/* RTC API */
int ds3231_init(struct ds3231 *obj, const struct ds3231_device *dev,
		int epoch_year, ds3231_alarm_callback_t cb);
void ds3231_exit(struct ds3231 *obj, const struct ds3231_device *dev);
int ds3231_read_time(struct ds3231 *obj, struct rtc_time *tm);
int ds3231_set_time(struct ds3231 *obj, struct rtc_time *tm);
int ds3231_set_alarm(struct ds3231 *obj);
int ds3231_read_alarm(struct ds3231 *obj);
int ds3231_enable_alarm(struct ds3231 *obj);
int ds3231_disable_alarm(struct ds3231 *obj);
int ds3231_toggle_alarm(struct ds3231 *obj, bool alarm_enabled);

#endif /* DRIVERS_DS3231_H */
