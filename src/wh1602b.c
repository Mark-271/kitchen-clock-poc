#include <libopencm3/stm32/gpio.h>
#include <wh1602b.h>
#include <common.h>
#include <delay.h>

/**
 * WH1602B initalization function.
 * @param wh Structure whose fields should be filled previously by the caller
 * @return 0 if success or -2 when failure
 */
int wh1602b_init(struct wh1602b *wh)
{
	return 0;
}

void wh1602b_exit(struct wh1602b *wh)
{
	UNUSED(wh);
}
