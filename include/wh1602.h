#ifndef WH1602_H
#define WH1602_H

struct wh1602 {
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

int wh1602_init(struct wh1602 *wh);
void wh1602_exit(struct wh1602 *wh);
void wh1602_display_clear(struct wh1602 *wh);
void wh1602_return_home(struct wh1602 *wh);
void wh1602_entry_mode_set(struct wh1602 *wh, bool id, bool sh);
void wh1602_display_on_off(struct wh1602 *wh, bool d, bool c, bool b);
void wh1602_function_set(struct wh1602 *wh, bool dl, bool n, bool f);
void wh1602_set_addr_ddram(struct wh1602 *wh, uint8_t addr);
void wh1602_write_char(struct wh1602 *wh, uint8_t data);
void wh1602_print_str(struct wh1602 *wh, char *str);
void wh1602_erase_screen(struct wh1602 *wh);
#endif /* WH1602_H */
