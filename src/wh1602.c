#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <string.h>
#include <board.h>
#include <common.h>
#include <delay.h>
#include <wh1602.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/cortex.h>

#define DB4_MASK			0x1
#define DB5_MASK			0x2
#define DB6_MASK			0x4
#define DB7_MASK			0x8
#define DB_PINS_MASK			WH1602_DB4_PIN | WH1602_DB5_PIN | \
					WH1602_DB6_PIN | WH1602_DB7_PIN

#define ENABLE_PULSE_DELAY		1
#define SET_POWER_DELAY			20
#define WAIT_TIME_DELAY			5
#define EXEC_TIME_DELAY			50
#define DISPLAY_CLEAN_RETURN_DELAY	2

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
static void wh1602_write_nibble(struct wh1602 *wh, uint8_t nibble)
{
	uint16_t pins;

	enter_critical(flags);
	pins = gpio_get(wh->port, DB_PINS_MASK);
	exit_critical(flags);

	pins &= ~(DB_PINS_MASK);
	if (nibble & BIT(0))
		pins |= wh->db4;
	if (nibble & BIT(1))
		pins |= wh->db5;
	if (nibble & BIT(2))
		pins |= wh->db6;
	if (nibble & BIT(3))
		pins |= wh->db7;

	enter_critical(flags);
	gpio_port_write(wh->port, pins);
	exit_critical(flags);
}

/**
 * Clear Display
 *
 * Clear all the display data by writing "20H" to all DDRAM address
 * Set DDRAM address to "00H" into AC
 * Return cursor to the original status
 * Make entry mode increment
 *
 * @param wh Structure whose fields should be filled by the caller
 */
void wh1602_display_clear(struct wh1602 *wh)
{
	printf("TESTING %s()...\n", __func__);

	gpio_clear(wh->port, wh->rs | DB_PINS_MASK);
	wh1602_en_pulse(wh);

	gpio_set(wh->port, wh->db4);
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
 * @param wh Structure whose fields should be filled by the caller
 */
void wh1602_return_home(struct wh1602 *wh)
{
	printf("TESTING %s()...\n", __func__);

	gpio_clear(wh->port, wh->rs | DB_PINS_MASK);
	wh1602_en_pulse(wh);

	gpio_set(wh->port, wh->db5);
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
 * @param id Boolean. When "true", cursor/blink moves to right.
 * 	     DDRAM address is increased by 1
 * @param sh Boolean. When "true", and DDRAM write operation, shift of display
 * 	     is performed according to @ref id value.
 */
void wh1602_entry_mode_set(struct wh1602 *wh, bool id, bool sh)
{
	printf("TESTING %s()...\n", __func__);

	gpio_clear(wh->port, wh->rs | DB_PINS_MASK);
	wh1602_en_pulse(wh);

	gpio_set(wh->port, wh->db6);
	if (id)
		gpio_set(wh->port, wh->db5);
	if (sh)
		gpio_set(wh->port, wh->db4);
	wh1602_en_pulse(wh);

	enter_critical(flags);
	udelay(EXEC_TIME_DELAY);
	exit_critical(flags);
}

/**
 * Display ON/OFF
 *
 * Control display/cursor/blink ON/OFF 1 bit register
 *
 * @param wh Structure whose fields should be filled by the caller
 * @param d Boolean. When "true", entire display is turned on
 * @param c Boolean. When "true", cursor is turned on
 * @param b Boolean. When "true", cursor blink is on
 */
void wh1602_display_on_off(struct wh1602 *wh, bool d, bool c, bool b)
{
	printf("TESTING %s()...\n", __func__);

	gpio_clear(wh->port, wh->rs | DB_PINS_MASK);
	wh1602_en_pulse(wh);

	gpio_set(wh->port, wh->db7);
	if (d)
		gpio_set(wh->port, wh->db6);
	if (c)
		gpio_set(wh->port, wh->db5);
	if (b)
		gpio_set(wh->port, wh->db4);
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
 * @param dl Boolean. Interface data length control bit.
 * 	     When "false" 4-bit bus mode enabled
 * @param n Boolean. Display line number control bit
 * 	    When "true" 2-line display mode is set
 * @param f Boolean. Dislay font type control bit
 * 	    When "false" 5x8 dots format display mode.
 */
void wh1602_function_set(struct wh1602 *wh, bool dl, bool n, bool f)
{
	printf("TESTING %s()...\n", __func__);

	gpio_clear(wh->port, wh->rs | DB_PINS_MASK);

	gpio_set(wh->port, wh->db5);
	if (dl)
		gpio_set(wh->port, wh->db4);
	wh1602_en_pulse(wh);

	gpio_clear(wh->port, wh->db5 | wh->db4);
	if (n)
		gpio_set(wh->port, wh->db7);
	if (f)
		gpio_set(wh->port, wh->db6);
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

	enter_critical(flags);
	mdelay(SET_POWER_DELAY);
	exit_critical(flags);

	gpio_clear(wh->port, wh->en | wh->rs | DB_PINS_MASK);
	ret = gpio_get(wh->port, wh->en | wh->rs | DB_PINS_MASK);
	if (ret) {
		printf("0x%x\n", ret);
		return -1;
	}

	wh1602_function_set(wh, true, false, false);
	enter_critical(flags);
	mdelay(WAIT_TIME_DELAY);
	exit_critical(flags);

	wh1602_function_set(wh, true, false, false);
	enter_critical(flags);
	mdelay(WAIT_TIME_DELAY);
	exit_critical(flags);

	wh1602_function_set(wh, true, false, false);
	wh1602_function_set(wh, false, true, false);
	wh1602_display_on_off(wh, false, false, false);
	wh1602_display_clear(wh);
	wh1602_entry_mode_set(wh, true, false);
	wh1602_display_on_off(wh, true, true, true);

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

	printf("TESTING %s()...\n", __func__);

	gpio_clear(wh->port, wh->rs);
	wh1602_write_nibble(wh, temp >> 4);
	wh1602_en_pulse(wh);
	wh1602_write_nibble(wh, temp & 0x0f);
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
	printf("TESTING %s()...\n", __func__);

	wh1602_write_nibble(wh, data >> 4);
	gpio_set(wh->port, wh->rs);
	wh1602_en_pulse(wh);

	wh1602_write_nibble(wh, data & 0x0f);
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
