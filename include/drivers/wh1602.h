/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 *         Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef DRIVERS_WH1602_H
#define DRIVERS_WH1602_H

#include <tools/common.h>
#include <stdint.h>

#define CURSOR_BLINK_OFF	0
#define CURSOR_BLINK_ON		0x01
#define CURSOR_MOVE_LEFT	0
#define CURSOR_MOVE_RIGHT	0x02
#define CURSOR_OFF		0
#define CURSOR_ON		0x02
#define DATA_BUS_4		0
#define DATA_BUS_8		0x10
#define DISABLE_LCD_SHIFT	0
#define ENABLE_DISPLAY_SHIFT	0x1
#define FOMT_TYPE_5_11		0x1
#define FONT_TYPE_5_8		0
#define LCD_1_LINE_MODE		0
#define LCD_2_LINE_MODE		0x08
#define LCD_OFF			0
#define LCD_ON			0x04
#define LINE_1			0
#define LINE_2			0x1

struct wh1602_gpio {
	/* GPIO port */
	uint32_t port;

	/* GPIO pins */
	uint16_t rs;
	uint16_t en;
	uint16_t db4;
	uint16_t db5;
	uint16_t db6;
	uint16_t db7;
};

struct wh1602 {
	/* User data */
	struct wh1602_gpio gpio;

	/* Internal driver's data */
	uint16_t pin_mask;	/* cached value: db4 | db5 | db6 | db7 */
	uint16_t lookup[9];	/* mapping: data bit -> GPIO line */
};

int wh1602_init(struct wh1602 *obj, const struct wh1602_gpio *gpio);
void wh1602_exit(struct wh1602 *obj);
void wh1602_clear_display(struct wh1602 *obj);
void wh1602_return_home(struct wh1602 *obj);
void wh1602_set_entry_mode(struct wh1602 *obj, int id, int sh);
void wh1602_control_display(struct wh1602 *obj, int d, int c, int b);
void wh1602_set_function(struct wh1602 *obj, int dl, int n, int f);
void wh1602_set_address(struct wh1602 *obj, uint8_t addr);
void wh1602_write_char(struct wh1602 *obj, uint8_t data);
void wh1602_print_str(struct wh1602 *obj, const char *str);
void wh1602_set_line(struct wh1602 *obj, int line);

#endif /* DRIVERS_WH1602_H */
