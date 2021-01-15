#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <board.h>
#include <common.h>
#include <delay.h>
#include <wh1602.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/cortex.h>

#define ENABLE_PULSE_DELAY		1
#define SET_POWER_DELAY			20
#define WAIT_TIME_DELAY			5
#define EXEC_TIME_DELAY			50
#define DISPLAY_CLEAN_RETURN_DELAY	2

#define WH_LOOKUP_GPIO(obj, nibble)					\
	(obj->lookup[nibble & BIT(0)] | obj->lookup[nibble & BIT(1)] |	\
	 obj->lookup[nibble & BIT(2)] | obj->lookup[nibble & BIT(3)])

static unsigned long flags;

/* Control chip enable signal */
static void wh1602_en_pulse(struct wh1602 *wh)
{
	gpio_set(wh->port, wh->en);

	enter_critical(flags);
	udelay(ENABLE_PULSE_DELAY);
	exit_critical(flags);

	gpio_clear(wh->port, wh->en);

	enter_critical(flags);
	udelay(ENABLE_PULSE_DELAY);
	exit_critical(flags);
}

/* Write 4-bit data. RS signal mode should be controlled by the caller */
static void wh1602_write(struct wh1602 *wh, uint8_t nibble)
{
	uint16_t data;

	enter_critical(flags);
	data = gpio_get(wh->port, wh->pin_mask);
	exit_critical(flags);

	data &= ~wh->pin_mask;
	data |= WH_LOOKUP_GPIO(wh, nibble);

	enter_critical(flags);
	gpio_port_write(wh->port, data);
	exit_critical(flags);
}

/**
 * Clear Display
 *
 * Clear all the display data by writing "20H" to all DDRAM addresses
 * Set DDRAM address to "00H" into AC
 * Return cursor to the original status
 * Make entry mode increment
 *
 * @param wh Structure fields should be filled by the caller
 */
void wh1602_display_clear(struct wh1602 *wh)
{
	uint8_t data = CLEAR_DISPLAY;

	gpio_clear(wh->port, wh->rs);

	wh1602_write(wh, data  >> 4);
	wh1602_en_pulse(wh);

	wh1602_write(wh, data & 0x0f);
	wh1602_en_pulse(wh);

	enter_critical(flags);
	mdelay(DISPLAY_CLEAN_RETURN_DELAY);
	exit_critical(flags);
}

/**
 * Return Home
 *
 * Set DDRAM address to "00H"  into address counter
 * Return cursor to its original site
 * Return display to its original status, if shifted
 * Contents of DDRAM doesn't chabge
 *
 * @param wh Structure fields should be filled by the caller
 */
void wh1602_return_home(struct wh1602 *wh)
{
	uint8_t data = RETURN_HOME;

	gpio_clear(wh->port, wh->rs);

	wh1602_write(wh, data  >> 4);
	wh1602_en_pulse(wh);

	wh1602_write(wh, data & 0x0f);
	wh1602_en_pulse(wh);

	enter_critical(flags);
	mdelay(DISPLAY_CLEAN_RETURN_DELAY);
	exit_critical(flags);
}

/**
 * Entry Mode Set
 *
 * Set the moving direction of cursor and display
 *
 * @param wh Structure whose fields should be filled by the caller
 * @param id Cursor moving control bit. DDRAM address is increased by 1
 * @param sh When set and DDRAM write operation, shift of display
 * 	     is performed according to @ref id value
 */
void wh1602_entry_mode_set(struct wh1602 *wh, enum lcd_cmd_bit id,
			   enum lcd_cmd_bit sh)
{
	uint8_t data = ENTRY_MODE_SET | id | sh;

	gpio_clear(wh->port, wh->rs);

	wh1602_write(wh, data  >> 4);
	wh1602_en_pulse(wh);

	wh1602_write(wh, data & 0x0f);
	wh1602_en_pulse(wh);

	enter_critical(flags);
	udelay(EXEC_TIME_DELAY);
	exit_critical(flags);
}

/**
 * Display ON/OFF control
 *
 * Control display/cursor/blink ON/OFF 1 bit register
 *
 * @param wh Structure whose fields should be filled by the caller
 * @param d When set, entire display is turned on
 * @param c When set, cursor is turned on
 * @param b When set, cursor blink is on
 */
void wh1602_display_control(struct wh1602 *wh, enum lcd_cmd_bit d,
			   enum lcd_cmd_bit c, enum lcd_cmd_bit b)
{
	uint8_t data = DISPLAY_CONTROL | d | c | b;

	gpio_clear(wh->port, wh->rs);

	wh1602_write(wh, data  >> 4);
	wh1602_en_pulse(wh);

	wh1602_write(wh, data & 0x0f);
	wh1602_en_pulse(wh);

	enter_critical(flags);
	udelay(EXEC_TIME_DELAY);
	exit_critical(flags);
}

/**
 * Function Set
 *
 * Contol data bus length, display line number and font size
 *
 * @param wh Structe should be filled by the caller
 * @param dl Interface data length control bit.
 * @param n  Display line number control bit.
 * @param f  Dislay font type control bit.
 */
void wh1602_function_set(struct wh1602 *wh, enum lcd_cmd_bit dl, enum lcd_cmd_bit n, enum lcd_cmd_bit f)
{
	uint8_t data = FUNC_SET | dl | n | f;

	gpio_clear(wh->port, wh->rs);

	wh1602_write(wh, data  >> 4);
	wh1602_en_pulse(wh);

	wh1602_write(wh, data & 0x0f);
	wh1602_en_pulse(wh);

	enter_critical(flags);
	udelay(EXEC_TIME_DELAY);
	exit_critical(flags);
}

/**
 * WH1602B initalization function
 *
 * @param wh Structure whose fields should be filled by the caller
 * @return 0 if success or -1 in the case of failure
 */
int wh1602_init(struct wh1602 *wh)
{
	int ret;

	gpio_clear(wh->port, wh->en | wh->rs | wh->pin_mask);
	ret = gpio_get(wh->port, wh->en | wh->rs | wh->pin_mask);
	if (ret)
		return -1;

	wh->pin_mask = wh->db4 | wh->db5 | wh->db6 | wh->db7;
	wh->lookup[0] = 0;
	wh->lookup[1] = wh->db4;
	wh->lookup[2] = wh->db5;
	wh->lookup[4] = wh->db6;
	wh->lookup[8] = wh->db7;

	enter_critical(flags);
	mdelay(SET_POWER_DELAY);
	exit_critical(flags);

	wh1602_function_set(wh, DB8, OFF, OFF);
	enter_critical(flags);
	mdelay(WAIT_TIME_DELAY);
	exit_critical(flags);

	wh1602_function_set(wh, DB8, OFF, OFF);
	enter_critical(flags);
	mdelay(WAIT_TIME_DELAY);
	exit_critical(flags);

	wh1602_function_set(wh, DB8, OFF, OFF);
	wh1602_function_set(wh, DB4, LINE_NUM_2, FONT_T_8);
	wh1602_display_control(wh, OFF, OFF, OFF);
	wh1602_display_clear(wh);
	wh1602_entry_mode_set(wh, CURSOR_MOV_R, OFF);
	wh1602_display_control(wh, DISPLAY_ON, CURSOR_ON, BLINK_ON);

	return 0;
}

/* Destroy object */
void wh1602_exit(struct wh1602 *wh)
{
	UNUSED(wh);
}

/**
 * Set DDRAM address in address counter
 *
 * @param wh Structure whose fields should be filled previously by the caller
 * @param addr Unsigned int8. DDRAM address
 */
void wh1602_set_addr_ddram(struct wh1602 *wh, uint8_t addr)
{
	uint8_t temp = addr | 0x80;

	gpio_clear(wh->port, wh->rs);
	wh1602_write(wh, temp >> 4);
	wh1602_en_pulse(wh);
	wh1602_write(wh, temp & 0x0f);
	wh1602_en_pulse(wh);

	enter_critical(flags);
	udelay(EXEC_TIME_DELAY);
	exit_critical(flags);
}

/**
 * Write data into internal RAM (DDRAM or CGRAM)
 *
 * @param wh Structure whose fields should be filled previously by the caller
 * @param data Unsigned int8. Data to be writen to RAM
 */
void wh1602_write_char(struct wh1602 *wh, uint8_t data)
{
	wh1602_write(wh, data >> 4);
	gpio_set(wh->port, wh->rs);
	wh1602_en_pulse(wh);

	wh1602_write(wh, data & 0x0f);
	gpio_set(wh->port, wh->rs);
	wh1602_en_pulse(wh);

	enter_critical(flags);
	udelay(EXEC_TIME_DELAY);
	exit_critical(flags);
}

/* Function to print string */
void wh1602_print_str(struct wh1602 *wh, char *str)
{
	while (*str)
		wh1602_write_char(wh, *str++);
}
/* Erase all data from screen */
void wh1602_erase_screen(struct wh1602 *wh)
{
	wh1602_display_clear(wh);
	wh1602_return_home(wh);
}
