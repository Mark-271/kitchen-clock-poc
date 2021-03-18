#include <drivers/rtc.h>
#include <drivers/i2c.h>
#include <tools/common.h>
#include <stdio.h>

#define RTC_CR			0x0e
#define RTC_SR			0x0f
#define RTC_SEC_REG		0x00
#define RTC_MIN_REG		0x01
#define RTC_HOUR_REG		0x02

static uint8_t dec2bcd(uint8_t val)
{
	return ((val / 10) << 4) | (val % 10);
}

static uint8_t bcd2dec(uint8_t val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

/**
 * Initialize RTC module.
 *
 * @param obj Device object
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
