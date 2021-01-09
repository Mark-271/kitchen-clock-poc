#include <libopencm3/stm32/gpio.h>
#include <wh1602b.h>
#include <common.h>
#include <delay.h>
#include <stdio.h>

/* Prints data bus values to serial console */
static void gpio_print(struct wh1602b *wh)
{
	int ret;
	ret = gpio_get(GPIOB, 0xf0);
	printf("0x%x\n", !!ret);
}

/* Control chip enable signal */
static void wh1602b_en_pulse(struct wh1602b *wh)
{
	gpio_set(wh->port, wh->en);
	udelay(1);
	gpio_clear(wh->port, wh->en);
	udelay(1);
}

/* Write 4-bit data. RS signal mode should be controlled by the caller */
static void wh1602b_write_nibble(struct wh1602b *wh, uint16_t nibble)
{;
	gpio_clear(wh->port, 0xf0);
	gpio_set(wh->port, nibble );
	wh1602b_en_pulse(wh);
}

void wh1602b_set_addr_ddram(struct wh1602b *wh, uint8_t addr)
{
	gpio_clear(wh->port, wh->rs);
	wh1602b_write_nibble(wh, addr & 0xf0);
	wh1602b_write_nibble(wh, (addr & 0x0f) << 4);
	udelay(50);
}

void wh1602b_write_data(struct wh1602b *wh, uint8_t data)
{
	gpio_set(wh->port, wh->rs);
	wh1602b_write_nibble(wh, data & 0xf0);
	wh1602b_write_nibble(wh, (data & 0x0f) << 4);
	udelay(50);
}

/**
 * WH1602B initalization function.
 * @param wh Structure whose fields should be filled previously by the caller
 * @return 0 if success or -2 when failure
 */
int wh1602b_init(struct wh1602b *wh)
{
	int ret;
	uint16_t pins_mask = wh->rs | wh->en | wh->db4 | wh->db5 | wh->db6 |
			     wh->db7;

	gpio_clear(wh->port, pins_mask);
	ret = gpio_get(wh->port, pins_mask);
	if (ret)
		return -1;

	mdelay(100);
	gpio_clear(wh->port, wh->rs);
	gpio_set(wh->port, wh->en);
	wh1602b_write_nibble(wh, 0x30);
	udelay(100);

	/*
	 * Function Set
	 * 0 0 0 0 1 DL N F * *
	 *  (DL=0, N=1, F=1)
	 */
	wh1602b_write_nibble(wh, 0x20);
	wh1602b_write_nibble(wh, 0xC0);
	udelay(100);

	/*
	 * Function Set
	 * 0 0 0 0 1 DL N F * *
	 *  (DL=0, N=1, F=1)
	 */
	wh1602b_write_nibble(wh, 0x20);
	wh1602b_write_nibble(wh, 0xC0);
	udelay(100);

	/*
	 * Display ONOFF Control
	 * 0 0 0 0 0 0 1 D C B
	 * (D=1, C=1, B=1)
	 */
	wh1602b_write_nibble(wh, 0x0);
	wh1602b_write_nibble(wh, 0xF0);
	udelay(100);

	/*
	 * Display Clear
	 * 0 0 0 0 0 0 0 0 0 1
	 */
	wh1602b_write_nibble(wh, 0x0);
	wh1602b_write_nibble(wh, 0x10);
	mdelay(5);

	/*
	 * Entry Mode Set
	 * 0 0 0 0 0 0 0 1 I/D SH
	 * (I/D=1, SH=0)
	 */
	wh1602b_write_nibble(wh, 0x0);
	wh1602b_write_nibble(wh, 0x60);
	udelay(50);

	return 0;
}

void wh1602b_exit(struct wh1602b *wh)
{
	UNUSED(wh);
}
