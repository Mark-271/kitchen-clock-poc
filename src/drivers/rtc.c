#include <drivers/rtc.h>
#include <drivers/i2c.h>
#include <tools/common.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define RTC_CR			0x0e
#define RTC_SR			0x0f
#define RTC_SECONDS		0x00	/* DS3231 seconds register address */
#define RTC_TM_BUF_LEN		7	/* Numner of date/time registers */

static uint8_t dec2bcd(uint8_t val)
{
	return ((val / 10) << 4) | (val % 10);
}

static uint8_t bcd2dec(uint8_t val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

 * Read date/time registers from RTC device.
 *
 * @param obj RTC device
 */
void rtc_get_date(struct rtc *obj)
{
	int ret;
	size_t i;
	uint8_t temp[RTC_TM_BUF_LEN];
	uint8_t buf[RTC_TM_BUF_LEN];
	ret = i2c_read_buf_poll(obj->addr, RTC_SECONDS, temp, RTC_TM_BUF_LEN);
	if (ret != 0)
		return;

	for (i = 0; i <  RTC_TM_BUF_LEN; i++)
		buf[i] = bcd2dec(temp[i]);
	memcpy(&obj->tm, &buf[0], RTC_TM_BUF_LEN);
}
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
