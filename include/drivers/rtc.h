#ifndef DRIVERS_RTC_H
#define DRIVERS_RTC_H

#include <libopencm3/stm32/exti.h>
#include <stdint.h>

typedef void (*rtc_callback_t)(void);

/* RTC time / date registers */
struct rtc_tm {
	uint8_t ss;
	uint8_t mm;
	uint8_t hh;
	uint8_t day;
	uint8_t date;
	uint8_t month;
	uint8_t year;
};

/* RTC hardware parameters */
struct rtc_device {
	uint32_t port;
	uint16_t pin;
	int irq;			/* exti number */
	enum exti_trigger_type trig; /* exti trigger condition */
	uint32_t i2c_base;
	uint8_t addr;
};

/* RTC API */
int rtc_init(const struct rtc_device *obj);
void rtc_exit(struct rtc_tm *obj);
void rtc_read_time(struct rtc_tm *obj);
void rtc_read_date(struct rtc_tm *obj);
int rtc_set_time(struct rtc_tm *obj);
int rtc_set_date(struct rtc_tm *obj);

#endif /* DRIVERS_RTC_H */
