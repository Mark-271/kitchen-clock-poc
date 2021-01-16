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
	uint16_t pin_mask;
	uint16_t lookup[9];
};

enum wh1602_cmd_base {
	CLEAR_DISPLAY 	 = 0x01,
	RETURN_HOME	 = 0x02,
	ENTRY_MODE_SET	 = 0x04,
	DISPLAY_CONTROL	 = 0x08,
	FUNC_SET	 = 0x20,
};

typedef enum lcd_cmd_bit {
	CURSOR_MOV_R	 = 0x02,
	CURSOR_MOV_L	 = 0,
	DISPLAY_SH_ON	 = 0x01,
	DISPLAY_ON	 = 0x04,
	CURSOR_ON	 = 0x02,
	BLINK_ON	 = 0x01,
	OFF		 = 0,
	DB8		 = 0x10,
	DB4		 = 0,
	LINE_NUM_2	 = 0x08,
	LINE_NUM_1	 = 0,
	FONT_T_8	 = 0,
	FONT_T_11	 = 0x04,
	LINE_1		 = 0,
	LINE_2		 = 0x01,
} lcd_cmd_bit;

int wh1602_init(struct wh1602 *wh);
void wh1602_exit(struct wh1602 *wh);
void wh1602_display_clear(struct wh1602 *wh);
void wh1602_return_home(struct wh1602 *wh);
void wh1602_entry_mode_set(struct wh1602 *wh, lcd_cmd_bit id, lcd_cmd_bit sh);
void wh1602_display_control(struct wh1602 *wh, lcd_cmd_bit d, lcd_cmd_bit c,
			    lcd_cmd_bit b);
void wh1602_function_set(struct wh1602 *wh, lcd_cmd_bit dl, lcd_cmd_bit n,
			 lcd_cmd_bit f);
void wh1602_set_addr_ddram(struct wh1602 *wh, uint8_t addr);
void wh1602_write_char(struct wh1602 *wh, uint8_t data);
void wh1602_print_str(struct wh1602 *wh, char *str);
void wh1602_erase_screen(struct wh1602 *wh);
void wh1602_set_line(struct wh1602 *wh, enum lcd_cmd_bit line);

#endif /* WH1602_H */
