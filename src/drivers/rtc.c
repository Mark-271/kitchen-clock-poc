#include <drivers/rtc.h>
#include <drivers/i2c.h>
#include <tools/common.h>
#include <stdio.h>

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
