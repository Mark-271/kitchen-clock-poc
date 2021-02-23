#include <drivers/wh1602.h>
#include <board.h>
#include <tools/common.h>
#include <libopencm3/stm32/gpio.h>
#include <stdio.h>
#include <string.h>

/* Necessary time delays */
#define ENABLE_PULSE_DELAY		1	/* usec */
#define SET_POWER_DELAY			20000	/* usec */
#define WAIT_TIME_DELAY			5000	/* usec */
#define EXEC_TIME_DELAY			50	/* usec */
#define DISPLAY_CLEAN_RETURN_DELAY	2000	/* usec */

/* Return corresponding GPIO lines for set bits in nibble */
#define WH_LOOKUP_GPIO(obj, nibble)					\
	(obj->lookup[nibble & BIT(0)] | obj->lookup[nibble & BIT(1)] |	\
	 obj->lookup[nibble & BIT(2)] | obj->lookup[nibble & BIT(3)])

/* Control bits of corresponding LCD instructions */
enum wh1602_cmd_control_bit {
	CLEAR_DISPLAY 	 = BIT(0),
	RETURN_HOME	 = BIT(1),
	ENTRY_MODE_SET	 = BIT(2),
	DISPLAY_CONTROL	 = BIT(3),
	FUNC_SET	 = BIT(5),
	SET_ADDRESS	 = BIT(7),
};

/**
 * Control chip enable signal.
 *
 * @note Caller must disable interrupts
 */
static void wh1602_en_pulse(struct wh1602 *obj)
{
	gpio_set(obj->gpio.port, obj->gpio.en);
	udelay(ENABLE_PULSE_DELAY);
	gpio_clear(obj->gpio.port, obj->gpio.en);
	udelay(ENABLE_PULSE_DELAY);
}

/**
 * Write 4-bit data.
 *
 * RS signal mode should be controlled by the caller.
 *
 * @note Caller must disable interrupts
 */
static void wh1602_write(struct wh1602 *obj, uint8_t nibble)
{
	uint16_t data;

	data = gpio_port_read(obj->gpio.port);
	data &= ~obj->pin_mask;
	data |= WH_LOOKUP_GPIO(obj, nibble);
	gpio_port_write(obj->gpio.port, data);
	wh1602_en_pulse(obj);
}

static void wh1602_write_cmd(struct wh1602 *obj, uint8_t cmd, unsigned delay_us)
{
	unsigned long flags;

	enter_critical(flags);
	gpio_clear(obj->gpio.port, obj->gpio.rs);
	wh1602_write(obj, cmd >> 4);
	wh1602_write(obj, cmd & 0x0f);
	udelay(delay_us);
	exit_critical(flags);
}

static void wh1602_write_data(struct wh1602 *obj, uint8_t data,
			      unsigned delay_us)
{
	unsigned long flags;

	enter_critical(flags);
	gpio_set(obj->gpio.port, obj->gpio.rs);
	wh1602_write(obj, data >> 4);
	wh1602_write(obj, data & 0x0f);
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
 * @param obj Structure fields should be filled by the caller
 */
void wh1602_clear_display(struct wh1602 *obj)
{
	wh1602_write_cmd(obj, CLEAR_DISPLAY, DISPLAY_CLEAN_RETURN_DELAY);
}

/**
 * Return home.
 *
 * Set DDRAM address to "00H"  into address counter.
 * Return cursor to its original site.
 * Return display to its original status, if shifted.
 * Contents of DDRAM doesn't change.
 *
 * @param obj Structure fields should be filled by the caller
 */
void wh1602_return_home(struct wh1602 *obj)
{
	wh1602_write_cmd(obj, RETURN_HOME, DISPLAY_CLEAN_RETURN_DELAY);
}

/**
 * Entry mode set.
 *
 * Set the moving direction of cursor and display.
 *
 * @param obj Structure whose fields should be filled by the caller
 * @param id Cursor moving control bit. DDRAM address is increased by 1
 * @param sh When set and DDRAM write operation, shift of display
 * 	     is performed according to @ref id value
 */
void wh1602_set_entry_mode(struct wh1602 *obj, int id, int sh)
{
	wh1602_write_cmd(obj, ENTRY_MODE_SET | id | sh, EXEC_TIME_DELAY);
}

/**
 * Display on/off control.
 *
 * Control display/cursor/blink ON/OFF 1 bit register.
 *
 * @param obj Structure whose fields should be filled by the caller
 * @param d When set, entire display is turned on
 * @param c When set, cursor is turned on
 * @param b When set, cursor blink is on
 */
void wh1602_control_display(struct wh1602 *obj, int d, int c, int b)
{
	wh1602_write_cmd(obj, DISPLAY_CONTROL | d | c | b, EXEC_TIME_DELAY);
}

/**
 * Function set.
 *
 * Contol data bus length, display line number and font size.
 *
 * @param obj Structe should be filled by the caller
 * @param dl Interface data length control bit.
 * @param n  Display line number control bit.
 * @param f  Dislay font type control bit.
 */
void wh1602_set_function(struct wh1602 *obj, int dl, int n, int f)
{
	wh1602_write_cmd(obj, FUNC_SET | dl | n | f, EXEC_TIME_DELAY);
}

/**
 * WH1602B initalization function.
 *
 * @param[out] wh Structure whose fields should be filled by the caller
 * @param[in] gpio GPIO info where LCD is connected
 * @return 0 if success or -1 in the case of failure
 */
int wh1602_init(struct wh1602 *obj, const struct wh1602_gpio *gpio)
{
	/* Fill struct fields */
	obj->gpio = *gpio;
	obj->pin_mask = obj->gpio.db4 | obj->gpio.db5 | obj->gpio.db6 |
			obj->gpio.db7;
	obj->lookup[0] = 0;
	obj->lookup[1] = obj->gpio.db4;
	obj->lookup[2] = obj->gpio.db5;
	obj->lookup[4] = obj->gpio.db6;
	obj->lookup[8] = obj->gpio.db7;

	/* Init pins state */
	gpio_clear(obj->gpio.port, obj->gpio.en | obj->gpio.rs | obj->pin_mask);

	/* Startup sequence by datasheet */
	udelay(SET_POWER_DELAY);
	wh1602_set_function(obj, DATA_BUS_8, LCD_1_LINE_MODE, FONT_TYPE_5_8);
	udelay(WAIT_TIME_DELAY);
	wh1602_set_function(obj, DATA_BUS_8, LCD_1_LINE_MODE, FONT_TYPE_5_8);
	udelay(WAIT_TIME_DELAY);
	wh1602_set_function(obj, DATA_BUS_8, LCD_1_LINE_MODE, FONT_TYPE_5_8);
	wh1602_set_function(obj, DATA_BUS_4, LCD_2_LINE_MODE, FONT_TYPE_5_8);
	wh1602_control_display(obj, LCD_OFF, CURSOR_OFF, CURSOR_BLINK_OFF);
	wh1602_clear_display(obj);
	wh1602_set_entry_mode(obj, CURSOR_MOVE_RIGHT, DISABLE_LCD_SHIFT);
	wh1602_control_display(obj, LCD_ON, CURSOR_ON, CURSOR_BLINK_ON);

	return 0;
}

/* Destroy object */
void wh1602_exit(struct wh1602 *obj)
{
	UNUSED(obj);
}

/**
 * Set DDRAM address in address counter.
 *
 * @param obj Structure whose fields should be filled previously by the caller
 * @param addr DDRAM address
 */
void wh1602_set_address(struct wh1602 *obj, uint8_t addr)
{
	wh1602_write_cmd(obj, addr | SET_ADDRESS, EXEC_TIME_DELAY);
}

/**
 * Write data into internal RAM (DDRAM or CGRAM).
 *
 * @param obj Structure whose fields should be filled previously by the caller
 * @param data Data to be writen to RAM
 */
void wh1602_write_char(struct wh1602 *obj, uint8_t data)
{
	wh1602_write_data(obj, data, EXEC_TIME_DELAY);
}

/* Function to print string */
/**
 * Print string of data on LCDa.
 *
 * @param obj LCD object
 * @param str String to print
 */
void wh1602_print_str(struct wh1602 *obj, const char *str)
{
	while (*str)
		wh1602_write_char(obj, *str++);
}

/**
 * Set line on LCD to print data.
 *
 * @param obj LCD object
 * @param line Line number to set (0 or 1)
 */
void wh1602_set_line(struct wh1602 *obj, int line)
{
	wh1602_set_address(obj, line == 0 ? 0x00 : 0x40);
}
