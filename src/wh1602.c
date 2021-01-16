#include <stdio.h>
#include <string.h>

#include <board.h>
#include <delay.h>
#include <wh1602.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/cm3/cortex.h>

#define ENABLE_PULSE_DELAY		1	/* usec */
#define SET_POWER_DELAY			20000	/* usec */
#define WAIT_TIME_DELAY			5000	/* usec */
#define EXEC_TIME_DELAY			50	/* usec */
#define DISPLAY_CLEAN_RETURN_DELAY	2000	/* usec */

#define WH_LOOKUP_GPIO(obj, nibble)					\
	(obj->lookup[nibble & BIT(0)] | obj->lookup[nibble & BIT(1)] |	\
	 obj->lookup[nibble & BIT(2)] | obj->lookup[nibble & BIT(3)])

/**
 * Control chip enable signal.
 *
 * @note Caller must disable interrupts
 */
static void wh1602_en_pulse(struct wh1602 *wh)
{
	gpio_set(wh->gpio.port, wh->gpio.en);
	udelay(ENABLE_PULSE_DELAY);
	gpio_clear(wh->gpio.port, wh->gpio.en);
	udelay(ENABLE_PULSE_DELAY);
}

/**
 * Write 4-bit data. RS signal mode should be controlled by the caller.
 *
 * @note Caller must disable interrupts
 */
static void wh1602_write(struct wh1602 *wh, uint8_t nibble)
{
	uint16_t data;

	data = gpio_port_read(wh->gpio.port);
	data &= ~wh->pin_mask;
	data |= WH_LOOKUP_GPIO(wh, nibble);
	gpio_port_write(wh->gpio.port, data);
	wh1602_en_pulse(wh);
}

static void wh1602_write_cmd(struct wh1602 *wh, uint8_t cmd, unsigned delay_us)
{
	unsigned long flags;

	enter_critical(flags);
	gpio_clear(wh->gpio.port, wh->gpio.rs);
	wh1602_write(wh, cmd >> 4);
	wh1602_write(wh, cmd & 0x0f);
	udelay(delay_us);
	exit_critical(flags);
}

static void wh1602_write_data(struct wh1602 *wh, uint8_t data,
			      unsigned delay_us)
{
	unsigned long flags;

	enter_critical(flags);
	gpio_set(wh->gpio.port, wh->gpio.rs);
	wh1602_write(wh, data >> 4);
	wh1602_write(wh, data & 0x0f);
	udelay(delay_us);
	exit_critical(flags);
}

/**
 * Clear display.
 *
 * Clear all the display data by writing "20H" to all DDRAM addresses.
 * Set DDRAM address to "00H" into AC.
 * Return cursor to the original status.
 * Make entry mode increment.
 *
 * @param wh Structure fields should be filled by the caller
 */
void wh1602_display_clear(struct wh1602 *wh)
{
	wh1602_write_cmd(wh, CLEAR_DISPLAY, DISPLAY_CLEAN_RETURN_DELAY);
}

/**
 * Return home.
 *
 * Set DDRAM address to "00H"  into address counter.
 * Return cursor to its original site.
 * Return display to its original status, if shifted.
 * Contents of DDRAM doesn't change.
 *
 * @param wh Structure fields should be filled by the caller
 */
void wh1602_return_home(struct wh1602 *wh)
{
	wh1602_write_cmd(wh, RETURN_HOME, DISPLAY_CLEAN_RETURN_DELAY);
}

/**
 * Entry mode set.
 *
 * Set the moving direction of cursor and display.
 *
 * @param wh Structure whose fields should be filled by the caller
 * @param id Cursor moving control bit. DDRAM address is increased by 1
 * @param sh When set and DDRAM write operation, shift of display
 * 	     is performed according to @ref id value
 */
void wh1602_entry_mode_set(struct wh1602 *wh, enum lcd_cmd_bit id,
			   enum lcd_cmd_bit sh)
{
	wh1602_write_cmd(wh, ENTRY_MODE_SET | id | sh, EXEC_TIME_DELAY);
}

/**
 * Display on/off control.
 *
 * Control display/cursor/blink ON/OFF 1 bit register.
 *
 * @param wh Structure whose fields should be filled by the caller
 * @param d When set, entire display is turned on
 * @param c When set, cursor is turned on
 * @param b When set, cursor blink is on
 */
void wh1602_display_control(struct wh1602 *wh, enum lcd_cmd_bit d,
			   enum lcd_cmd_bit c, enum lcd_cmd_bit b)
{
	wh1602_write_cmd(wh, DISPLAY_CONTROL | d | c | b, EXEC_TIME_DELAY);
}

/**
 * Function set.
 *
 * Contol data bus length, display line number and font size.
 *
 * @param wh Structe should be filled by the caller
 * @param dl Interface data length control bit.
 * @param n  Display line number control bit.
 * @param f  Dislay font type control bit.
 */
void wh1602_function_set(struct wh1602 *wh, enum lcd_cmd_bit dl,
			 enum lcd_cmd_bit n, enum lcd_cmd_bit f)
{
	wh1602_write_cmd(wh, FUNC_SET | dl | n | f, EXEC_TIME_DELAY);
}

/**
 * WH1602B initalization function.
 *
 * @param[out] wh Structure whose fields should be filled by the caller
 * @param[in] gpio GPIO info where LCD is connected
 * @return 0 if success or -1 in the case of failure
 */
int wh1602_init(struct wh1602 *wh, const struct wh1602_gpio *gpio)
{
	/* Fill struct fields */
	wh->gpio = *gpio;
	wh->pin_mask = wh->gpio.db4 | wh->gpio.db5 | wh->gpio.db6 | wh->gpio.db7;
	wh->lookup[0] = 0;
	wh->lookup[1] = wh->gpio.db4;
	wh->lookup[2] = wh->gpio.db5;
	wh->lookup[4] = wh->gpio.db6;
	wh->lookup[8] = wh->gpio.db7;

	/* Init pins state */
	gpio_clear(wh->gpio.port, wh->gpio.en | wh->gpio.rs | wh->pin_mask);

	/* Startup sequence by datasheet */
	udelay(SET_POWER_DELAY);
	wh1602_function_set(wh, DB8, OFF, OFF);
	udelay(WAIT_TIME_DELAY);
	wh1602_function_set(wh, DB8, OFF, OFF);
	udelay(WAIT_TIME_DELAY);
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
 * Set DDRAM address in address counter.
 *
 * @param wh Structure whose fields should be filled previously by the caller
 * @param addr DDRAM address
 */
void wh1602_set_addr_ddram(struct wh1602 *wh, uint8_t addr)
{
	wh1602_write_cmd(wh, addr | SET_ADDRESS, EXEC_TIME_DELAY);
}

/**
 * Write data into internal RAM (DDRAM or CGRAM).
 *
 * @param wh Structure whose fields should be filled previously by the caller
 * @param data Data to be writen to RAM
 */
void wh1602_write_char(struct wh1602 *wh, uint8_t data)
{
	wh1602_write_data(wh, data, EXEC_TIME_DELAY);
}

/* Function to print string */
void wh1602_print_str(struct wh1602 *wh, const char *str)
{
	while (*str)
		wh1602_write_char(wh, *str++);
}

/**
 * Set line on LCD to print data.
 *
 * @param wh LCD object
 * @param line Line number to set (0 or 1)
 */
void wh1602_set_line(struct wh1602 *wh, int line)
{
	wh1602_set_addr_ddram(wh, line == 0 ? 0x00 : 0x40);
}
