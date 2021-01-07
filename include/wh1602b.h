#ifndef WH1602B_H
#define WH1602B_H

struct wh1602b {
	uint32_t port;
	uint16_t rs;
	uint16_t en;
	uint16_t db4;
	uint16_t db5;
	uint16_t db6;
	uint16_t db7;
};

void wh1602b_init(struct wh1602b *wh,
		  uint32_t gpio_port,
		  uint16_t gpio_pin_rs,
		  uint16_t gpio_pin_en,
		  uint16_t gpio_pin_db4,
		  uint16_t gpio_pin_db5,
		  uint16_t gpio_pin_db6,
		  uint16_t gpio_pin_db7);

void wh1602b_exit(struct wh1602b *wh);

#endif /* WH1602B_H */
