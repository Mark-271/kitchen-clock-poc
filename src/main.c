#include <board.h>
#include <common.h>
#include <ds18b20.h>
#include <keyboard.h>
#include <serial.h>
#include <sched.h>
#include <wh1602.h>
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

#define SCAN_TEMPERATURE_DELAY	5000

static bool kbd_event_flag;
static bool ds18b20_presence_flag;
bool kbd_timer_event_flag = false;
static int showtemp_id; /* task ID */

static struct kbd kbd = {
	.port = KBD_GPIO_PORT,
	.l1 = KBD_GPIO_L1_PIN,
	.l2 = KBD_GPIO_L2_PIN,
	.r1 = KBD_GPIO_R1_PIN,
	.r2 = KBD_GPIO_R2_PIN
};
static struct ds18b20 ts = {
	.port = DS18B20_GPIO_PORT,
	.pin = DS18B20_GPIO_PIN,
};

static struct wh1602 wh;

static void show_temp(void *data)
{
	struct wh1602 *obj = (struct wh1602 *)(data);
	char buf[20];
	char *temper;
	ds18b20_presence_flag = false;

	ts.temp = ds18b20_read_temp(&ts);
	while (ts.temp.frac > 9)
		ts.temp.frac /= 10;
	temper = ds18b20_temp2str(&ts.temp, buf);
	wh1602_set_address(obj, 0x00);
	wh1602_print_str(obj, temper);
	mdelay(1000);

	sched_set_ready(showtemp_id);
}

static void handle_btn(enum kbd_btn btn, bool pressed)
{
	/* TODO: To implement f */
}
static void init(void)
{
	const char *str = "Poc Watch";
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

	board_init();
	serial_init(&serial);
	sched_init();

	kbd_init(&kbd, handle_btn);

	err = ds18b20_init(&ts);
	if (err)
		ds18b20_presence_flag = false;

	err = wh1602_init(&wh, &wh_gpio);
	if (err)
		hang();

	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, str);
	wh1602_control_display(&wh, LCD_ON, CURSOR_OFF, CURSOR_BLINK_OFF);

	err = sched_add_task("showtemp", show_temp, &wh, &showtemp_id);
	if (err < 0) {
		printf("Can't add task to show_temp\n");
		hang();
	}
	sched_set_ready(showtemp_id);

}

#if 0
static void __attribute__((__noreturn__)) loop(void)
{
	int c = 0;
	for (;;) {
		if (kbd_timer_event_flag) {
			kbd_event_flag = true;
			kbd_timer_event_flag = false;
		}

		if (kbd_event_flag) {
			kbd_event_flag = false;
			int val = kbd_push_button(&kbd);
			printf("%d\n", val);

			switch (val) {
			case KBD_BTN_1:
				wh1602_clear_display(&wh);
				break;
			case KBD_BTN_2:
				wh1602_write_char(&wh, c + '0');
				c = (c == 9) ? 0 : c + 1;
				break;
			case KBD_BTN_3:
				wh1602_set_line(&wh, LINE_1);
				ds18b20_presence_flag = true;
				break;
			case KBD_BTN_4:
				wh1602_set_line(&wh, LINE_2);
				ds18b20_presence_flag = true;
				break;
			default:
				puts("No press");
				break;
			}
		}

		if (ds18b20_presence_flag) {
			char buf[20];
			char *temper;
			ds18b20_presence_flag = false;

			ts.temp = ds18b20_read_temp(&ts);
			while (ts.temp.frac > 9)
				ts.temp.frac /= 10;
			temper = ds18b20_temp2str(&ts.temp, buf);
			wh1602_print_str(&wh, temper);
		}
	}
}
#endif
int main(void)
{
	init();
	sched_start();
}
