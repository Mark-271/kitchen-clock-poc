#include <logic.h>
#include <board.h>
#include <core/irq.h>
#include <core/log.h>
#include <core/sched.h>
#include <core/swtimer.h>
#include <drivers/buzzer.h>
#include <drivers/ds18b20.h>
#include <drivers/ds3231.h>
#include <drivers/kbd.h>
#include <drivers/serial.h>
#include <drivers/systick.h>
#include <drivers/wh1602.h>
#include <tools/common.h>
#include <tools/tools.h>
#include <libopencm3/stm32/gpio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define MENU_NUM		2
#define EPOCH_YEAR		2021 /* years */
#define GET_TEMP_DELAY		5000 /* msec */
#define BUF_LEN			25
#define TIM_PERIOD		1000 /* msec */

typedef void (*logic_handle_stage_func_t)(void);

static void logic_handle_btn(int btn, bool pressed);

/* Keep 0 as undefined state */
enum logic_stage {
	STAGE_UNDEFINED,
	STAGE_INIT,
	STAGE_MENU,
	STAGE_TIME,
	STAGE_TEMP,
	STAGE_EXIT,
	STAGE_NUM,
};

enum button {
	BUTTON_1,
	BUTTON_2,
	BUTTON_3,
	BUTTON_4,
};

static bool ds18b20_presence_flag = true;

static struct kbd kbd;
static struct rtc_time tm;
static struct ds3231 rtc;
static struct wh1602 wh;
static struct ds18b20 ts = {
	.port = DS18B20_GPIO_PORT,
	.pin = DS18B20_GPIO_PIN,
};
static struct swtimer_sw_tim swtim;		/* software timer object */

static const char * const menu[MENU_NUM] = {
	"1-Time ",
	"2-Temp",
};

/* Initialize peripheral drivers */
static void logic_init_drivers(void)
{
	int err;

	struct serial_params serial = {
		.uart = SERIAL_USART,
		.baud = CONFIG_SERIAL_SPEED,
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
	const struct ds3231_device device = {
		.port = DS3231_I2C_GPIO_PORT,
		.pin = DS3231_ALARM_PIN,
		.irq = DS3231_EXTI_IRQ,
		.trig = DS3231_EXTI_TRIGGER,
		.i2c_base = DS3231_I2C_BASE,
		.addr = DS3231_DEVICE_ADDR,
	};

	serial_init(&serial);

	err = systick_init();
	if (err) {
		pr_emerg("Error: Can't initialize systick: %d\n", err);
		hang();
	}

	err = kbd_init(&kbd, &kbd_gpio, logic_handle_btn);
	if (err) {
		pr_emerg("Error: Can't initialize kbd: %d\n", err);
		hang();
	}

	err = ds18b20_init(&ts);
	if (err) {
		pr_err("Error: Can't initialize ds18b20: %d\n", err);
		ds18b20_presence_flag = false;
	}

	err = wh1602_init(&wh, &wh_gpio);
	if (err) {
		pr_emerg("Error: Can't initialize wh1602: %d\n", err);
		hang();
	}

	err = ds3231_init(&rtc, &device, EPOCH_YEAR);
	if (err) {
		pr_emerg("Error: Can't initialize RTC device DS3231: %d\n", err);
		hang();
	}

	buzz_init(&buzz, BUZZ_GPIO_PORT, BUZZ_GPIO_PIN);
}

static char *logic_measure_temper(void *data)
{
	struct ds18b20 *obj = (struct ds18b20 *)(data);
	char buf[BUF_LEN];

	obj->temp = ds18b20_read_temp(&ts);

	while (obj->temp.frac > 9)
		obj->temp.frac /= 10;

	return ds18b20_temp2str(&obj->temp, buf);
}

static void logic_handle_temper(void)
{
	char *temper;

	/* If ds18b20 is out of order, the program should skip it */
	if (!ds18b20_presence_flag)
		return;

	temper = logic_measure_temper(&ts);

	wh1602_clear_display(&wh);
	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, temper);
}

static void logic_handle_time(void)
{
	swtimer_tim_start(swtim.id);
}

static void logic_handle_timer(void *data)
{
	int err;
	char buf[BUF_LEN];
	struct ds3231 *obj = (struct ds3231 *)(data);
	struct tm *t;

	err = ds3231_read_time(obj, &tm);
	if (err) {
		pr_emerg("Error: Can't read time from ds3231\n");
		hang();
	}

	t = (struct tm *)(&tm);

	time2str(t, buf);
	wh1602_clear_display(&wh);
	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, buf);
}

static void logic_handle_menu(void)
{
	size_t i;

	wh1602_control_display(&wh, LCD_ON, CURSOR_OFF, CURSOR_BLINK_OFF);
	wh1602_clear_display(&wh);
	wh1602_set_line(&wh, LINE_1);

	for (i = 0; i < MENU_NUM; i++)
		wh1602_print_str(&wh, menu[i]);
}

static void logic_handle_init(void)
{
	int ret;

	swtim.cb = logic_handle_timer;
	swtim.data = &rtc;
	swtim.period = TIM_PERIOD;

	logic_init_drivers();

	ret = swtimer_tim_register(&swtim);
	if (ret < 0) {
		pr_emerg("Error: Can't register timer\n");
		hang();
	}
	swtimer_tim_stop(swtim.id);

	logic_handle_menu();
}

static void logic_handle_exit(void)
{
	logic_handle_menu();
}

logic_handle_stage_func_t logic_stage_handler[STAGE_NUM] = {
	NULL,
	logic_handle_init,
	logic_handle_menu,
	logic_handle_time,
	logic_handle_temper,
	logic_handle_exit,
};

static void logic_handle_stage(enum logic_stage stage)
{
	logic_handle_stage_func_t transition_func;

	transition_func = logic_stage_handler[stage];
	if (!transition_func) {
		pr_emerg("Error: Transition function doesn't exist for"
			 "stage %d\n", stage);
		hang();
	}

	transition_func();
}

static void logic_handle_btn(int button, bool pressed)
{
	wh1602_clear_display(&wh);

	if (button == BUTTON_1 && pressed) {
		logic_handle_stage(STAGE_TIME);
	} else if ((button == BUTTON_1) && !pressed) {
		swtimer_tim_stop(swtim.id);
		logic_handle_stage(STAGE_MENU);
	} else if ((button == BUTTON_2) && pressed) {
		logic_handle_stage(STAGE_TEMP);
	} else if (!pressed) {
		logic_handle_stage(STAGE_MENU);
	}
}

void logic_start(void)
{
	logic_handle_stage(STAGE_INIT);
}
