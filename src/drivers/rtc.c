#include <drivers/rtc.h>
#include <drivers/i2c.h>
#include <tools/common.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define RTC_CR			0x0e
#define RTC_SR			0x0f
#define RTC_SECONDS		0x00	/* DS3231 seconds register address */
#define RTC_TM_BUF_LEN		4	/* Number of time registers */
#define RTC_DT_BUF_LEN		3	/* Quantity of date registers */

static uint8_t dec2bcd(uint8_t val)
{
	return ((val / 10) << 4) | (val % 10);
}

static uint8_t bcd2dec(uint8_t val)
{
	return (val & 0x0f) + (val >> 4) * 10;
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
int rtc_read_time(struct rtc *obj)
{
	int ret;
	size_t i;
	uint8_t temp[RTC_TM_BUF_LEN];
	uint8_t buf[RTC_TM_BUF_LEN];

	ret = i2c_read_buf_poll(obj->addr, RTC_SECONDS, temp, RTC_TM_BUF_LEN);
	if (ret != 0)
		return ret;

	for (i = 0; i <  RTC_TM_BUF_LEN; i++)
		buf[i] = bcd2dec(temp[i]);

	memcpy(&obj->tm, &buf[0], RTC_TM_BUF_LEN);

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
int rtc_read_date(struct rtc *obj)
{
	int ret;
	size_t i;
	uint8_t temp[RTC_DT_BUF_LEN];
	uint8_t buf[RTC_DT_BUF_LEN];

	ret = i2c_read_buf_poll(obj->addr, RTC_SECONDS, temp, RTC_DT_BUF_LEN);
	if (ret != 0)
		return ret;

	for (i = 0; i <  RTC_DT_BUF_LEN; i++)
		buf[i] = bcd2dec(temp[i]);

	memcpy(&obj->tm, &buf[0], RTC_DT_BUF_LEN);

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
int rtc_set_time(struct rtc *obj, uint8_t hh, uint8_t mm, uint8_t ss)
{
	int ret;
	uint8_t buf[3];

	cm3_assert(ss < 60);
	cm3_assert(mm < 60);
	cm3_assert(hh < 24);

	buf[0] = dec2bcd(ss);
	buf[1] = dec2bcd(mm);
	buf[2] = dec2bcd(hh);

	ret = i2c_write_buf_poll(obj->addr, RTC_SECONDS, buf, 3);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * Initialize real-time clock device.
 *
 * @param obj RTC device
 * @param base I2C base
 * @param addr Address of device
 */
int rtc_init(struct rtc *obj, uint32_t base, uint8_t addr)
{
	int ret;

	obj->i2c_base = base;
	obj->addr = addr;

	i2c_init(obj->i2c_base);
	ret = i2c_detect_device(obj->addr);
	if (ret != 0)
		return ret;

	return 0;
}

/**
 * De-initialize software timer framework.
 */
void rtc_exit(struct rtc *obj)
{
	UNUSED(obj);
}
