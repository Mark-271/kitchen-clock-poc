#include <drivers/ds3231.h>
#include <drivers/i2c.h>
#include <tools/bcd.h>
#include <tools/common.h>
#include <core/irq.h>
#include <core/sched.h>
#include <tools/tools.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/* DS3231 registers */
#define RTC_CR			0x0e	/* Config register */
#define RTC_SR			0x0f	/* Status register */
#define RTC_SECONDS		0x00	/* Register of seconds */
#define RTC_DAY			0x03	/* Day offset register */
#define RTC_BUF_LEN		7	/* Number of data registers */
#define RTC_TASK		"ds3231"
#define RTC_A1F			BIT(0) /* Alarm 1 flag */

#define TM_START_YEAR		1900
#define MIN_TM_YEAR		0
#define MIN_REGS_YEAR		0
#define MAX_REGS_YEAR		99
#define TM_MDAYS		30

struct ds3231_regs {
	uint8_t ss;	/* 0-59 */
	uint8_t mm;	/* 0-59 */
	uint8_t hh;	/* 0-23 */
	uint8_t day;	/* 1-7 */
	uint8_t date;	/* 1-31 */
	uint8_t month;	/* 1-12 + century */
	uint8_t year;	/* 0-99 */
};

struct ds3231_alarm {
	int task_id;
	struct irq_action action;
	struct ds3231_regs time;
	ds3231_callback_t cb;
};

/* Driver structure */
struct ds3231 {
	int epoch_year;
	struct ds3231_device device;
	struct ds3231_alarm alarm;
	struct ds3231_regs regs;
};

/* Singleton driver object */
static struct ds3231 ds3231;

/* -------------------------------------------------------------------------- */

static bool ds3231_time2regs(const struct rtc_time *tm,
			     struct ds3231_regs *regs)
{
	const int regs_year = tm->tm_year + TM_START_YEAR - ds3231.epoch_year;

	if (regs_year > MAX_REGS_YEAR || regs_year < MIN_REGS_YEAR)
		return false;

	regs->ss	= dec2bcd(tm->tm_sec);
	regs->mm	= dec2bcd(tm->tm_min);
	regs->hh	= dec2bcd(tm->tm_hour);
	regs->day	= tm->tm_wday + 1;
	regs->date	= dec2bcd(tm->tm_mday);
	regs->month	= dec2bcd(tm->tm_mon + 1);
	regs->year	= dec2bcd(regs_year);

	return true;
}

static bool ds3231_regs2time(const struct ds3231_regs *regs,
			     struct rtc_time *tm)
{
	const int year = bcd2dec(regs->year) + ds3231.epoch_year;
	const int tm_year = year - TM_START_YEAR;

	if (tm_year < MIN_TM_YEAR)
		return false;

	tm->tm_sec	= bcd2dec(regs->ss);
	tm->tm_min	= bcd2dec(regs->mm);
	tm->tm_hour	= bcd2dec(regs->hh);
	tm->tm_mday	= bcd2dec(regs->date);
	tm->tm_mon	= bcd2dec(regs->month - 1);
	tm->tm_year	= tm_year;
	tm->tm_wday	= regs->day - 1;
	tm->tm_yday	= get_yday(tm->tm_mon, tm->tm_mday, tm->tm_year) - 1;
	tm->tm_isdst	= 0;

	return true;
}

static void ds3231_exti_init(struct ds3231 *obj)
{
	nvic_enable_irq(obj->device.irq);
	exti_select_source(obj->device.pin, obj->device.port);
	exti_set_trigger(obj->device.pin, obj->device.trig);
	exti_enable_request(obj->device.pin);
}

static irqreturn_t ds3231_exti_isr(int irq, void *data)
{
	struct ds3231 *obj = (struct ds3231 *)(data);

	UNUSED(irq);

	nvic_disable_irq(obj->device.irq);
	sched_set_ready(obj->alarm.task_id);

	return IRQ_HANDLED;
}

static void ds3231_task(void *data)
{
	int ret;
	uint8_t buf[] = {~RTC_A1F};

	struct ds3231 *obj = (struct ds3231 *)(data);

	ret = i2c_write_buf_poll(obj->device.addr, RTC_SR, buf, 1);
	if (ret != 0) {
		printf("Error %d: Can't write data\n", ret);
		return;
	}

	nvic_enable_irq(obj->device.irq);
	obj->alarm.cb();
}

/**
 * Read time/date registers from ds3231 device.
 *
 * @param[out] tm Structure used to store time/date values
 * @return 0 on success or negative value on error
 */
int ds3231_read_time(struct rtc_time *tm)
{
	int ret;
	bool res;
	uint8_t buf[RTC_BUF_LEN];

	ret = i2c_read_buf_poll(ds3231.device.addr, RTC_SECONDS, buf,
				RTC_BUF_LEN);
	if (ret != 0)
		return ret;

	ds3231.regs.ss		= buf[0];
	ds3231.regs.mm		= buf[1];
	ds3231.regs.hh		= buf[2];
	ds3231.regs.day		= buf[3];
	ds3231.regs.date	= buf[4];
	ds3231.regs.month	= buf[5];
	ds3231.regs.year	= buf[6];

	res = ds3231_regs2time(&ds3231.regs, tm);
	if (!res)
		return -1;

	return 0;
}

/**
 * Set time/date to ds3231.
 *
 * Write data to ds3231 time/date registers.
 * 24-hour mode is selected by default.
 *
 * @param[in] tm Structure used to store time/date values. Should be
 * 		 filled by caller.
 * @return 0 on success or negative value on error
 */
int ds3231_set_time(struct rtc_time *tm)
{
	int ret;
	bool res;
	uint8_t buf[RTC_BUF_LEN];

	res = ds3231_time2regs(tm, &ds3231.regs);
	if (!res)
		return -1;

	buf[0] = ds3231.regs.ss;
	buf[1] = ds3231.regs.mm;
	buf[2] = ds3231.regs.hh;
	buf[3] = ds3231.regs.day;
	buf[4] = ds3231.regs.date;
	buf[5] = ds3231.regs.month;
	buf[6] = ds3231.regs.year;

	ret = i2c_write_buf_poll(ds3231.device.addr, RTC_SECONDS,
				 buf,RTC_BUF_LEN);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * Initialize real-time clock device.
 *
 * @param obj RTC device
 * @param epoch_year Min year value which RTC can be set up with; >= 1900
 * @return 0 on success or negative value on error
 */
int ds3231_init(const struct ds3231_device *obj, int epoch_year)
{
	int ret;

	ds3231.device = *obj;
	ds3231.epoch_year = epoch_year;

	ds3231.alarm.action.handler = ds3231_exti_isr;
	ds3231.alarm.action.irq = ds3231.device.irq;
	ds3231.alarm.action.name = RTC_TASK;
	ds3231.alarm.action.data = &ds3231;

	ret = gpio2irq(ds3231.device.pin);
	if (ret < 0)
		return -1;
	ds3231.device.irq = ret;

	i2c_init(ds3231.device.i2c_base);
	ret = i2c_detect_device(ds3231.device.addr);
	if (ret != 0)
		return ret;

	ds3231_exti_init(&ds3231);

	ret = irq_request(&ds3231.alarm.action);
	if (ret != 0)
		return ret;

	ret = sched_add_task(RTC_TASK, ds3231_task, &ds3231,
			     &ds3231.alarm.task_id);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * De-initialize ds3231 device.
 */
void ds3231_exit(const struct ds3231_device *obj)
{
	UNUSED(obj);

	sched_del_task(ds3231.alarm.task_id);
	irq_free(&ds3231.alarm.action);
}
