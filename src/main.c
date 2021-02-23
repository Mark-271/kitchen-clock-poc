#include <board.h>
#include <core/irq.h>
#include <core/sched.h>
#include <drivers/ds18b20.h>
#include <drivers/kbd.h>
#include <drivers/serial.h>
#include <drivers/wh1602.h>
#include <tools/common.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/timer.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define GET_TEMPERATURE_DELAY	1000
#define LCD_GREETING_DELAY	2000

static int showtemp_id; /* task ID */

static struct kbd kbd;
static struct wh1602 wh;
static struct ds18b20 ts = {
	.port = DS18B20_GPIO_PORT,
	.pin = DS18B20_GPIO_PIN,
};

static void show_temp(void *data)
{
	struct wh1602 *obj = (struct wh1602 *)(data);
	char buf[20];
	char *temper;

	ts.temp = ds18b20_read_temp(&ts);
	while (ts.temp.frac > 9)
		ts.temp.frac /= 10;
	temper = ds18b20_temp2str(&ts.temp, buf);
	wh1602_set_address(obj, 0x00);
	wh1602_print_str(obj, temper);
	mdelay(GET_TEMPERATURE_DELAY);

	sched_set_ready(showtemp_id);
}

static void handle_btn(int button, bool pressed)
{
	if (pressed) {
		wh1602_write_char(&wh, button + '1');
		wh1602_write_char(&wh, 'p');
	} else if (!pressed) {
		wh1602_write_char(&wh, button + '1');
		wh1602_write_char(&wh, 'r');
	}
}

static void show_lcd(const char *greeting)
{
	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, greeting);
	wh1602_control_display(&wh, LCD_ON, CURSOR_OFF, CURSOR_BLINK_OFF);
	mdelay(LCD_GREETING_DELAY);
	wh1602_clear_display(&wh);
}

static void init(void)
{
	bool ds18b20_presence_flag = true;
	const char *greeting = "Poc Watch";
	int err;
	struct serial_params serial = {
		.uart = SERIAL_USART,
		.baud = 115200,
		.bits = 8,
		.stopbits = USART_STOPBITS_1,
		.parity = USART_PARITY_NONE,
		.mode = USART_MODE_TX_RX,
		.flow_control = USART_FLOWCONTROL_NONE
	};
	struct wh1602_gpio wh_gpio = {
		.port = WH1602_GPIO_PORT,
		.rs = WH1602_RS_PIN,
		.en = WH1602_EN_PIN,
		.db4 = WH1602_DB4_PIN,
		.db5 = WH1602_DB5_PIN,
		.db6 = WH1602_DB6_PIN,
		.db7 = WH1602_DB7_PIN,
	};
	struct kbd_gpio kbd_gpio = {
		.port = KBD_GPIO_PORT,
		.read[0] = KBD_GPIO_L1_PIN,
		.read[1] = KBD_GPIO_L2_PIN,
		.scan[0] = KBD_GPIO_R1_PIN,
		.scan[1] = KBD_GPIO_R2_PIN,
		.trigger = KBD_EXTI_TRIGGER,
	};

	board_init();
	serial_init(&serial);
	sched_init();

	err = kbd_init(&kbd, &kbd_gpio, handle_btn);
	if (err) {
		printf("Can't initialize kbd\n");
		hang();
	}

	err = ds18b20_init(&ts);
	if (err) {
		printf("Can't initialize ds18b20\n");
		ds18b20_presence_flag = false;
	}

	err = wh1602_init(&wh, &wh_gpio);
	if (err) {
		printf("Can't initialize wh1602\n");
		hang();
	}
	show_lcd(greeting);

	/* If ds18b20 is out of order, the program should skip it */
	if (ds18b20_presence_flag) {
		err = sched_add_task("showtemp", show_temp, &wh, &showtemp_id);
		if (err < 0) {
			printf("Can't add task to show_temp\n");
			hang();
		}
		sched_set_ready(showtemp_id);
	}
}

int main(void)
{
	init();
	irq_init();
	sched_start();
}
