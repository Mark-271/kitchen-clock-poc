#include <libopencm3/stm32/gpio.h>
#include <wh1602b.h>
#include <common.h>
#include <delay.h>

static void display_init(struct wh1602b *wh)
{
	/* TODO: Implement function */
}

void wh1602b_init(struct wh1602b *wh, uint32_t gpio_port,
		  uint16_t gpio_pin_rs, uint16_t gpio_pin_en,
		  uint16_t gpio_pin_db4, uint16_t gpio_pin_db5,
		  uint16_t gpio_pin_db6, uint16_t gpio_pin_db7)
{
	wh->port = gpio_port;
	wh->rs = gpio_pin_rs;
	wh->en = gpio_pin_en;
	wh->db4 = gpio_pin_db4;
	wh->db5 = gpio_pin_db5;
	wh->db6 = gpio_pin_db6;
	wh->db7 = gpio_pin_db7;

	display_init(wh);
}

void wh1602b_exit(struct wh1602b *wh)
{
	UNUSED(wh);
}
