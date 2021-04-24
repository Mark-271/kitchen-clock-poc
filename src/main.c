#include <board.h>
#include <core/irq.h>
#include <core/sched.h>
#include <core/swtimer.h>
#include <drivers/ds18b20.h>
#include <drivers/ds3231.h>
#include <drivers/kbd.h>
#include <drivers/serial.h>
#include <drivers/systick.h>
#include <drivers/wh1602.h>
#include <tools/common.h>
#include <libopencm3/stm32/gpio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define LCD_GREETING_DELAY	2000 /* msec */
#define GET_TEMP_DELAY		5000 /* msec */
#define DS3231_GET_SEC_DELAY	1000 /* msec */
#define EPOCH_YEAR		2021 /* years */

static struct kbd kbd;
static struct rtc_time tm;
static struct ds3231 rtc;
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
	wh1602_set_line(obj, LINE_2);
	wh1602_print_str(obj, temper);
	wh1602_set_line(obj, LINE_1);
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
	const struct swtimer_hw_tim hw_tim = {
		.base = SWTIMER_TIM_BASE,
		.irq = SWTIMER_TIM_IRQ,
		.rst = SWTIMER_TIM_RST,
		.arr = SWTIMER_TIM_ARR_VAL,
		.psc = SWTIMER_TIM_PSC_VAL,
	};
	const struct ds3231_device device = {
		.port = DS3231_I2C_GPIO_PORT,
		.pin = DS3231_ALARM_PIN,
		.irq = DS3231_EXTI_IRQ,
		.trig = DS3231_EXTI_TRIGGER,
		.i2c_base = DS3231_I2C_BASE,
		.addr = DS3231_DEVICE_ADDR,
	};

	irq_init();
	board_init();
	serial_init(&serial);
	sched_init();

	err = systick_init();
	if (err) {
		printf("Can't initialize systick\n");
		hang();
	}

	err = swtimer_init(&hw_tim);
	if (err) {
		printf("Can't initialize swtimer\n");
		hang();
	}

	err = kbd_init(&kbd, &kbd_gpio, handle_btn);
	if (err) {
		printf("Can't initialize kbd\n");
		hang();
	}

	err = ds18b20_init(&ts);
	if (err) {
		printf("Can't initialize ds18b20: %d\n", err);
		ds18b20_presence_flag = false;
	}

	err = wh1602_init(&wh, &wh_gpio);
	if (err) {
		printf("Can't initialize wh1602\n");
		hang();
	}

	err = ds3231_init(&rtc, &device, EPOCH_YEAR);
	if (err) {
		printf("Can't initialize RTC device DS3231: %d\n", err);
		hang();
	}

	show_lcd(greeting);

	/* If ds18b20 is out of order, the program should skip it */
	if (ds18b20_presence_flag) {
		int id;

		id = swtimer_tim_register(show_temp, &wh, GET_TEMP_DELAY);
		if (id < 0) {
			printf("Unable to register swtimer\n");
			hang();
		}
	}
}

int main(void)
{
	init();
	sched_start();
}
