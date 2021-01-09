#ifndef WH1602B_H
#define WH1602B_H

struct wh1602b {
	/* GPIO port */
	uint32_t port;

	/* All the folllowing fiels are GPIO pins */
	uint16_t rs;
	uint16_t en;
	uint16_t db4;
	uint16_t db5;
	uint16_t db6;
	uint16_t db7;
};

int wh1602b_init(struct wh1602b *wh);
void wh1602b_exit(struct wh1602b *wh);
void wh1602b_set_addr_ddram(struct wh1602b *wh, uint8_t addr);
void wh1602b_write_data(struct wh1602b *wh, uint8_t data);
#endif /* WH1602B_H */
