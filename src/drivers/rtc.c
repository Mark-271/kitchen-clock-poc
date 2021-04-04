/* TODO:
 * Setup alarm.
 * Add and Fix comments
 */

#include <drivers/rtc.h>
#include <drivers/i2c.h>
#include <tools/bcd.h>
#include <tools/common.h>
#include <core/irq.h>
#include <core/sched.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define RTC_CR			0x0e	/* Config register */
#define RTC_SR			0x0f	/* Status register */
#define RTC_SECONDS		0x00	/* Register of seconds */
#define RTC_TM_BUF_LEN		4	/* Number of time registers */
#define RTC_DAY			0x03	/* Day offset register */
#define RTC_BUF_LEN		7
#define RTC_DT_BUF_LEN		4	/* Quantity of date registers */
#define RTC_TASK		"rtc"
#define RTC_A1F			(1 << 0) /* Alarm 1 flag */

struct rtc_alarm {
	int task_id;
	struct irq_action action;
	struct rtc_tm time;
	rtc_callback_t cb;
};

/* Driver structure */
struct rtc {
	struct rtc_device device;
	struct rtc_alarm alarm;
};

/* Singleton driver object */
static struct rtc rtc;

/* -------------------------------------------------------------------------- */

static void rtc_exti_init(struct rtc *obj)
{
	nvic_enable_irq(obj->device.irq);
	exti_select_source(obj->device.pin, obj->device.port);
	exti_set_trigger(obj->device.pin, obj->device.trig);
	exti_enable_request(obj->device.pin);
}

static irqreturn_t rtc_exti_isr(int irq, void *data)
{
	struct rtc *obj = (struct rtc *)(data);

	UNUSED(irq);

	nvic_disable_irq(obj->device.irq);
	sched_set_ready(obj->alarm.task_id);

	return IRQ_HANDLED;
}

static void rtc_task(void *data)
{
	int ret;
	uint8_t buf[] = {~RTC_A1F};

	struct rtc *obj = (struct rtc *)(data);

	ret = i2c_write_buf_poll(rtc.device.addr, RTC_SR, buf, ARRAY_SIZE(buf));
	if (ret != 0) {
		printf("Error %d: Can't write data\n", ret);
		return;
	}

	nvic_enable_irq(obj->device.irq);
	obj->alarm.cb();
}

/**
 * Read time registers from RTC device.
 *
 * Read values contained in time specific DS3231 registers:
 * seconds, minutes, hours, day of the week.
 *
 * @param obj RTC device
 * @return 0 on success or negative value on error
 */
int rtc_read_time(struct rtc_tm *tm)
{
	int ret;
	size_t i;
	uint8_t temp[RTC_TM_BUF_LEN];
	uint8_t buf[RTC_TM_BUF_LEN];
	struct rtc_device *obj = &rtc.device;

	ret = i2c_read_buf_poll(obj->addr, RTC_SECONDS, temp, RTC_TM_BUF_LEN);
	if (ret != 0)
		return ret;

	for (i = 0; i <  RTC_TM_BUF_LEN; i++)
		buf[i] = bcd2dec(temp[i]);

	memcpy(&tm, &buf[0], RTC_TM_BUF_LEN);

	return 0;
}

/**
 * Read date registers from RTC device.
 *
 * Read values contained inside DS3231 date registers (day, month, year).
 *
 * @param obj RTC device
 * @return 0 on success or negative value on failure
 */
int rtc_read_date(struct rtc_tm *tm)
{
	int ret;
	size_t i;
	uint8_t temp[RTC_DT_BUF_LEN];
	uint8_t buf[RTC_DT_BUF_LEN];
	struct rtc_device *obj = &rtc.device;

	ret = i2c_read_buf_poll(obj->addr, RTC_DATE, temp, RTC_DT_BUF_LEN);
	if (ret != 0)
		return ret;

	for (i = 0; i <  RTC_DT_BUF_LEN; i++)
		buf[i] = bcd2dec(temp[i]);

	memcpy(&tm, &buf[0], RTC_DT_BUF_LEN);

	return 0;
}

/**
 * Set time to RTC.
 *
 * Write time values to corresponding registers of DS3231
 * (seconds, minutes, hours).
 * 24-hour mode is selected by default.
 *
 * @param obj RTC device
 * @param ss Seconds
 * @param mm Minutes
 * @param hh Hours
 * @return 0 on success or negative value on error
 */
int rtc_set_time(struct rtc_tm *tm)
{
	int ret;
	uint8_t buf[3];
	struct rtc_device *obj = &rtc.device;

	cm3_assert(tm->ss < 60);
	cm3_assert(tm->mm < 60);
	cm3_assert(tm->hh < 24);

	buf[0] = dec2bcd(tm->ss);
	buf[1] = dec2bcd(tm->mm);
	buf[2] = dec2bcd(tm->hh);

	ret = i2c_write_buf_poll(obj->addr, RTC_SECONDS, buf, 3);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * Set date to rtc.
 *
 * Write date to corresponding registers (day of week/month, month, year)
 *
 * @param[in] tm Structure used to store time/date values. Should be
 * 		 filled by caller.
 * @return 0 on success or negative value on error
 */
int rtc_set_date(struct rtc_tm *tm)
{
	int ret;
	uint8_t buf[RTC_DT_BUF_LEN];

	struct rtc_device *obj = &rtc.device;

	cm3_assert(tm->day >= 1 && tm->day <= 7);
	cm3_assert(tm->date >= 1 && tm->date <= 31);
	cm3_assert(tm->month >= 1 && tm->month <= 12);
	cm3_assert(tm->year < 100);

	buf[0] = dec2bcd(tm->day);
	buf[1] = dec2bcd(tm->date);
	buf[2] = dec2bcd(tm->month);
	buf[2] = dec2bcd(tm->year);

	ret = i2c_write_buf_poll(obj->addr, RTC_DAY, buf, RTC_DT_BUF_LEN);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * Initialize real-time clock device.
 *
 * @param obj RTC device
 */
int rtc_init(const struct rtc_device *obj)
{
	int ret;

	rtc.device = *obj;

	rtc.alarm.action.handler = rtc_exti_isr;
	rtc.alarm.action.irq = rtc.device.irq;
	rtc.alarm.action.name = RTC_TASK;
	rtc.alarm.action.data = &rtc;

	ret = gpio2irq(rtc.device.pin);
	if (ret < 0)
		return -1;
	rtc.device.irq = ret;

	i2c_init(rtc.device.i2c_base);
	ret = i2c_detect_device(rtc.device.addr);
	if (ret != 0)
		return ret;

	rtc_exti_init(&rtc);

	ret = irq_request(&rtc.alarm.action);
	if (ret != 0)
		return ret;

	ret = sched_add_task(RTC_TASK, rtc_task, &rtc, &rtc.alarm.task_id);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * De-initialize rtc.
 */
void rtc_exit(struct rtc_tm *obj)
{
	sched_del_task(rtc.alarm.task_id);
	irq_free(&rtc.alarm.action);
	UNUSED(obj);
	UNUSED(rtc);
}
