#include <drivers/ds3231.h>
#include <drivers/i2c.h>
#include <core/log.h>
#include <core/sched.h>
#include <tools/bcd.h>
#include <tools/common.h>
#include <tools/tools.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

/* DS3231 registers */
#define DS3231_CR		0x0e	/* DS3231 Control register */
#define DS3231_SR		0x0f	/* DS3231 Status register */
#define DS3231_SECONDS		0x00	/* DS3231 Register of seconds */
#define DS3231_DAY		0x03	/* DS3231Day offset register */
#define DS3231_BUF_LEN		7	/* Number of data registers */
#define DS3231_TASK		"ds3231"
#define DS3231_ALARM1		0x07	/* DS3231 Alarm 1 offset register */
#define DS3231_ALARM1_EN	0x1d	/* Enable Alarm 1 */
#define DS3231_ALARM1_DIS	0x1c	/* Disable Alarm 1 */
#define DS3231_A1F_CLEAR	0x88	/* DS3231 Alarm 1 flag disabled */
#define DS3231_ALARM_MASK	0x80	/* DS3231 Alarm 1 mask */
#define ALARM1_BUF_LEN		4
#define MIN_TM_YEAR		0
#define MIN_REGS_YEAR		0
#define MAX_REGS_YEAR		99
#define TM_MDAYS		30
#define DS3231_INTCN		BIT(2)	/* Interrupt control bit */
#define DS3231_A1IE		BIT(0)	/* Alarm 1 interrupt enable bit */
#define DS3231_A1F		BIT(0)	/* Alarm 1 flag */
#define DS3231_A1M		BIT(7)	/* Alarm 1 mask bit */

/* ------------------------------------------------------------------------- */

static bool ds3231_time2regs(struct ds3231 *obj, const struct rtc_time *tm,
			     struct ds3231_regs *regs)
{
	const int regs_year = tm->tm_year + TM_START_YEAR - obj->epoch_year;

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

static bool ds3231_regs2time(struct ds3231 *obj, const struct ds3231_regs *regs,
			     struct rtc_time *tm)
{
	const int year = bcd2dec(regs->year) + obj->epoch_year;
	const int tm_year = year - TM_START_YEAR;

	if (tm_year < MIN_TM_YEAR)
		return false;

	tm->tm_sec	= bcd2dec(regs->ss);
	tm->tm_min	= bcd2dec(regs->mm);
	tm->tm_hour	= bcd2dec(regs->hh);
	tm->tm_mday	= bcd2dec(regs->date);
	tm->tm_mon	= bcd2dec(regs->month) - 1;
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

	exti_disable_request(obj->device.pin);
	nvic_disable_irq(obj->device.irq);
	sched_set_ready(obj->alarm.task_id);

	return IRQ_HANDLED;
}

static void ds3231_task(void *data)
{
	int ret;
	uint8_t buf;

	struct ds3231 *obj = (struct ds3231 *)(data);

	ret = i2c_read_buf_poll(obj->device.addr, DS3231_SR, &buf, 1);
	if (ret != 0) {
		pr_err("Error: Can't read DS3231_SR register: %d\n", ret);
		hang();
	}

	buf &= ~DS3231_A1F;

	ret = i2c_write_buf_poll(obj->device.addr, DS3231_SR, &buf, 1);
	if (ret != 0) {
		pr_err("Error: Can't write to DS3231_SR register: %d\n", ret);
		hang();
	}

	ret = ds3231_toggle_alarm(obj, false);
	if (ret) {
		pr_err("Error: Can't handle DS32321_CR register: %d\n", ret);
		hang();
	}

	nvic_enable_irq(obj->device.irq);
	exti_enable_request(obj->device.pin);

	obj->alarm.cb();
}

/**
 * Read time/date registers from ds3231 device.
 *
 * @param obj DS3231 device object
 * @param[out] tm Structure used to store time/date values
 * @return 0 on success or negative value on error
 */
int ds3231_read_time(struct ds3231 *obj, struct rtc_time *tm)
{
	int ret;
	bool res;
	uint8_t buf[DS3231_BUF_LEN];

	ret = i2c_read_buf_poll(obj->device.addr, DS3231_SECONDS, buf,
				DS3231_BUF_LEN);
	if (ret != 0)
		return ret;

	/*
	 * TODO: Find root cause and fix bug.
	 * DS3231 registers store trash values during first reading.
	 * Temporary fix is to add one more i2c_read_buf_poll()
	 */
	ret = i2c_read_buf_poll(obj->device.addr, DS3231_SECONDS, buf,
				DS3231_BUF_LEN);
	if (ret != 0)
		return ret;

	obj->regs.ss	= buf[0];
	obj->regs.mm	= buf[1];
	obj->regs.hh	= buf[2];
	obj->regs.day	= buf[3];
	obj->regs.date	= buf[4];
	obj->regs.month	= buf[5];
	obj->regs.year	= buf[6];

	res = ds3231_regs2time(obj, &obj->regs, tm);
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
 * @param obj DS3231 device object
 * @param[in] tm Structure used to store time/date values. Should be
 * 		filled by caller.
 * @return 0 on success or negative value on error
 */
int ds3231_set_time(struct ds3231 *obj, struct rtc_time *tm)
{
	int ret;
	bool res;
	uint8_t buf[DS3231_BUF_LEN];

	res = ds3231_time2regs(obj, tm, &obj->regs);
	if (!res)
		return -1;

	buf[0] = obj->regs.ss;
	buf[1] = obj->regs.mm;
	buf[2] = obj->regs.hh;
	buf[3] = obj->regs.day;
	buf[4] = obj->regs.date;
	buf[5] = obj->regs.month;
	buf[6] = obj->regs.year;

	ret = i2c_write_buf_poll(obj->device.addr, DS3231_SECONDS, buf,
				 DS3231_BUF_LEN);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * Turn on/off DS3231 Alarm 1.
 *
 * Interrupt control bit of DS3231 control register should be asserted.
 *
 * @param ob DS3231 device object
 * @param alarm_enabled Flag showing whether to enable or stop the alarm
 *
 * @return 0 on success or megative value on error
 */
int ds3231_toggle_alarm(struct ds3231 *obj, bool alarm_enabled)
{
	int ret;
	uint8_t buf;

	ret = i2c_read_buf_poll(obj->device.addr, DS3231_CR, &buf, 1);
	if (ret != 0)
		return ret;

	cm3_assert(buf & DS3231_INTCN); /* XXX */

	if (alarm_enabled) {
		obj->alarm.status = true;
		buf |= DS3231_A1IE;
	} else {
		obj->alarm.status = false;
		buf &= ~DS3231_A1IE;
	}

	ret = i2c_write_buf_poll(obj->device.addr, DS3231_CR, &buf, 1);
	if (ret != 0) {
		obj->alarm.status = false;
		return ret;
	}

	return 0;
}

/* Turn on alarm */
int ds3231_enable_alarm(struct ds3231 *obj)
{
	int err;
	uint8_t buf = DS3231_ALARM1_EN;

	err = i2c_write_buf_poll(obj->device.addr, DS3231_CR, &buf, 1);
	if (err)
		return err;

	obj->alarm.status = true;

	return 0;
}

/* Turn off alarm */
int ds3231_disable_alarm(struct ds3231 *obj)
{
	int err;
	uint8_t buf[] = {DS3231_ALARM1_DIS, DS3231_A1F_CLEAR};

	err = i2c_write_buf_poll(obj->device.addr, DS3231_CR, buf, 2);
	if (err)
		return err;

	obj->alarm.status = false;

	return 0;
}

/**
 * Set alarm.
 *
 * DS3231 contains two alarms. Only Alarm1 is in use. Alarm occures when
 * hours, minutes and seconds match. Day of a week or date are ignored.
 * Time data to trigger alarm should be set by caller.
 *
 * @param obj Device object
 * @return 0 on success or negative value on error
 */
int ds3231_set_alarm(struct ds3231 *obj)
{
	int err;
	bool res;
	uint8_t buf[ALARM1_BUF_LEN];

	res = ds3231_time2regs(obj, &obj->alarm.time, &obj->regs);
	if (!res)
		return -1;

	buf[0] = obj->regs.ss;
	buf[1] = obj->regs.mm;
	buf[2] = obj->regs.hh;
	buf[3] = obj->regs.date | DS3231_A1M;

	err = i2c_write_buf_poll(obj->device.addr, DS3231_ALARM1,
				 buf, ALARM1_BUF_LEN);
	if (err)
		return err;

	return 0;
}

/**
 * Read alarm time.
 *
 * Read data (minutes and hours) contained in alarm1 offset registers.
 *
 * @param obj Device object
 * @return 0 on success or negative value on error
 */
int ds3231_read_alarm(struct ds3231 *obj)
{
	int ret;
	int res;
	uint8_t buf[ALARM1_BUF_LEN];

	ret = i2c_read_buf_poll(obj->device.addr, DS3231_ALARM1, buf,
				ALARM1_BUF_LEN);
	if (ret != 0)
		return ret;

	obj->regs.mm = buf[1];
	obj->regs.hh = buf[2];

	res = ds3231_regs2time(obj, &obj->regs, &obj->alarm.time);
	if (!res)
		return -1;

	return 0;
}

/**
 * Initialize real-time clock device.
 *
 * @param obj DS3231 object
 * @param dev DS3231 device hardware parameters
 * @param epoch_year Min year value which DS3231 can be set up with; >= 1900
 * @param cb Callback to call when alarm triggers
 * @return 0 on success or negative value on error
 */
int ds3231_init(struct ds3231 *obj, const struct ds3231_device *dev,
		int epoch_year, ds3231_alarm_callback_t cb)
{
	int ret;

	obj->device = *dev;
	obj->epoch_year = epoch_year;

	obj->alarm.action.handler = ds3231_exti_isr;
	obj->alarm.action.irq = obj->device.irq;
	obj->alarm.action.name = DS3231_TASK;
	obj->alarm.action.data = obj;

	obj->alarm.cb = cb;

	ret = gpio2irq(obj->device.pin);
	if (ret < 0)
		return -1;

	obj->device.irq = ret;

	i2c_init(obj->device.i2c_base);
	ret = i2c_detect_device(obj->device.addr);
	if (ret != 0)
		return ret;

	ds3231_exti_init(obj);

	ret = irq_request(&obj->alarm.action);
	if (ret != 0)
		return ret;

	ret = sched_add_task(DS3231_TASK, ds3231_task, obj,
			     &obj->alarm.task_id);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * De-initialize ds3231 device.
 */
void ds3231_exit(struct ds3231 *obj, const struct ds3231_device *dev)
{
	UNUSED(obj);
	UNUSED(dev);

	sched_del_task(obj->alarm.task_id);
	irq_free(&obj->alarm.action);
}
