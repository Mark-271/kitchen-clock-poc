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
#include <drivers/melody.h>
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

#define MENU_NUM		3
#define ALARM_SYMBOL_POS	0x0f
#define ALARM_INDICATOR		0x2a
#define EPOCH_YEAR		2021	/* years */
#define GET_TEMP_DELAY		5000	/* msec */
#define BUF_LEN			25
#define TIM_PERIOD		5000	/* msec */
#define TEMPER_DISPLAY_ADDR	0x07

typedef void (*logic_handle_stage_func_t)(void);

/* Keep 0 as undefined state */
enum logic_stage {
	STAGE_UNDEFINED,
	STAGE_INIT,
	STAGE_MAIN_SCREEN,
	STAGE_MAIN_MENU,
	STAGE_TIME_SET_HH,
	STAGE_TIME_INCR_HH,
	STAGE_TIME_SET_MM,
	STAGE_TIME_INCR_MM,
	STAGE_ALARM,
	STAGE_ALARM_TOGGLE,
	STAGE_ALARM_INCR_HH,
	STAGE_ALARM_INCR_MM,
	STAGE_DATE_SET_WDAY,
	STAGE_DATE_INCR_WDAY,
	STAGE_DATE_DECR_WDAY,
	STAGE_DATE_SET_MONTH,
	STAGE_DATE_INCR_MONTH,
	STAGE_DATE_DECR_MONTH,
	STAGE_DATE_SET_MDAY,
	STAGE_DATE_INCR_MDAY,
	STAGE_DATE_DECR_MDAY,
	STAGE_DATE_SET_YY,
	STAGE_DATE_INCR_YY,
	STAGE_DATE_DECR_YY,
	/* --- */
	STAGE_NUM
};

enum logic_event {
	EVENT_START,
	EVENT_LEFT,	/* left button */
	EVENT_RIGHT,	/* right button */
	EVENT_UP,	/* up button */
	EVENT_DOWN,	/* down button */
	/* --- */
	EVENT_NUM
};

struct logic {
	enum logic_stage stage; /* current state of FSM */
};

static void logic_handle_event(enum logic_event event);
static void logic_handle_btn(int btn, bool pressed);

static uint8_t menu_addr[MENU_NUM] = {
	0x00,
	0x0a,
	0x40,
};

static const char * const menu_msg[MENU_NUM] = {
	"U-Alarm",
	"L-Back",
	"D-Time settings",
};

static bool ds18b20_presence_flag = true;

static struct logic logic;
static struct kbd kbd;
static struct rtc_time tm;
static struct ds3231 rtc;
static struct wh1602 wh;
static struct buzz buzz;
static struct swtimer_sw_tim swtim;		/* software timer object */
static struct ds18b20 ts = {
	.port = DS18B20_GPIO_PORT,
	.pin = DS18B20_GPIO_PIN,
};

/**
 * Transition lookup table for Moore FSM
 */
static const enum logic_stage logic_transitions[STAGE_NUM][EVENT_NUM] = {
	{ /* STAGE_UNDEFINED */
		STAGE_INIT,		/* EVENT_START */
		0,			/* EVENT_LEFT */
		0,			/* EVENT_RIGHT */
		0,			/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_INIT */
		STAGE_MAIN_SCREEN,	/* EVENT_START */
		0,			/* EVENT_LEFT */
		0,			/* EVENT_RIGHT */
		0,			/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_MAIN_SCREEN */
		0,			/* EVENT_START */
		STAGE_MAIN_MENU,	/* EVENT_LEFT */
		STAGE_MAIN_MENU,	/* EVENT_RIGHT */
		STAGE_MAIN_MENU,	/* EVENT_UP */
		STAGE_MAIN_MENU,	/* EVENT_DOWN */
	},
	{ /* STAGE_MAIN_MENU */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		0,			/* EVENT_RIGHT */
		STAGE_ALARM,		/* EVENT_UP */
		STAGE_TIME_SET_HH	/* EVENT_DOWN */
	},
	{ /* STAGE_TIME_SET_HH */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		STAGE_TIME_SET_MM,	/* EVENT_RIGHT */
		STAGE_TIME_INCR_HH,	/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_TIME_INCR_HH */
		STAGE_TIME_SET_HH,	/* EVENT_START */
		0,			/* EVENT_LEFT */
		0,			/* EVENT_RIGHT */
		0,			/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_TIME_SET_MM */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		STAGE_TIME_SET_HH,	/* EVENT_RIGHT */
		STAGE_TIME_INCR_MM,	/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_TIME_INCR_MM */
		STAGE_TIME_SET_MM,	/* EVENT_START */
		0,			/* EVENT_LEFT */
		0,			/* EVENT_RIGHT */
		0,			/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_ALARM */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		STAGE_ALARM_TOGGLE,	/* EVENT_RIGHT */
		STAGE_ALARM_INCR_HH,	/* EVENT_UP */
		STAGE_ALARM_INCR_MM,	/* EVENT_DOWN */
	},
	{ /* STAGE_ALARM_TOGGLE */
		STAGE_ALARM,		/* EVENT_START */
		0,			/* EVENT_LEFT */
		0,			/* EVENT_RIGHT */
		0,			/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_ALARM_INCR_HH */
		STAGE_ALARM,		/* EVENT_START */
		0,			/* EVENT_LEFT */
		0,			/* EVENT_RIGHT */
		0,			/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_ALARM_INCR_MM */
		STAGE_ALARM,		/* EVENT_START */
		0,			/* EVENT_LEFT */
		0,			/* EVENT_RIGHT */
		0,			/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_TIME_SET_MM */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		STAGE_DATE_SET_WDAY,	/* EVENT_RIGHT */
		STAGE_TIME_INCR_MM,	/* EVENT_UP */
		0,			/* EVENT_DOWN */
	},
	{ /* STAGE_DATE_SET_WDAY */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		STAGE_DATE_SET_MONTH,	/* EVENT_RIGHT */
		STAGE_DATE_INCR_WDAY,	/* EVENT_UP */
		STAGE_DATE_DECR_WDAY,	/* EVENT_DOWN */
	},
	{ /* STAGE_DATE_SET_MONTH */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		STAGE_DATE_SET_MDAY,	/* EVENT_RIGHT */
		STAGE_DATE_INCR_MONTH,	/* EVENT_UP */
		STAGE_DATE_DECR_MONTH,	/* EVENT_DOWN */
	},
	{ /* STAGE_DATE_SET_MDAY */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		STAGE_DATE_SET_YY,	/* EVENT_RIGHT */
		STAGE_DATE_INCR_MDAY,	/* EVENT_UP */
		STAGE_DATE_DECR_MDAY,	/* EVENT_DOWN */
	},
	{ /* STAGE_DATE_SET_YY */
		0,			/* EVENT_START */
		STAGE_MAIN_SCREEN,	/* EVENT_LEFT */
		STAGE_TIME_SET_HH,	/* EVENT_RIGHT */
		STAGE_DATE_INCR_YY,	/* EVENT_UP */
		STAGE_DATE_DECR_YY,	/* EVENT_DOWN */
	},
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

static char *logic_read_temper(void *data)
{
	struct ds18b20 *obj = (struct ds18b20 *)(data);
	char buf[BUF_LEN];

	obj->temp = ds18b20_read_temp(&ts);

	while (obj->temp.frac > 9)
		obj->temp.frac /= 10;

	return ds18b20_temp2str(&obj->temp, buf);
}

static void logic_print2lcd(char *time, char *date, char *temp)
{
	wh1602_clear_display(&wh);

	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, time);

	wh1602_set_line(&wh, LINE_2);
	wh1602_print_str(&wh, date);

	/* If ds18b20 is out of order, the program should skip it */
	if (ds18b20_presence_flag) {
		wh1602_set_address(&wh, TEMPER_DISPLAY_ADDR);
		wh1602_write_char(&wh, 't');
		wh1602_print_str(&wh, temp);
	}

	if (rtc.alarm.status) {
		wh1602_set_address(&wh, ALARM_SYMBOL_POS);
		wh1602_write_char(&wh, ALARM_INDICATOR);
	}
}

/* Callback to register inside swtimer */
static void logic_show_main_screen(void *data)
{
	char *temper;
	char date[BUF_LEN];
	char time[BUF_LEN];
	int err;
	struct tm *t;

	UNUSED(data);

	err = ds3231_read_time(&rtc, &tm);
	if (err) {
		pr_emerg("Error: Can't read time from ds3231: %d\n", err);
		hang();
	}

	t = (struct tm *)(&tm);

	date2str(t, date);
	time2str(t, time);
	temper = logic_read_temper(&ts);

	if (logic.stage == STAGE_MAIN_SCREEN)
		logic_print2lcd(time, date, temper);
}

static void logic_handle_stage_main_screen(void)
{
	wh1602_control_display(&wh, LCD_ON, CURSOR_OFF, CURSOR_BLINK_OFF);
	swtimer_tim_start(swtim.id);
}

static void logic_handle_stage_main_menu(void)
{
	size_t i;

	swtimer_tim_stop(swtim.id);
	wh1602_clear_display(&wh);

	for (i = 0; i < MENU_NUM; i++) {
		wh1602_set_address(&wh, menu_addr[i]);
		wh1602_print_str(&wh, menu_msg[i]);
	}
}

static void logic_handle_stage_init(void)
{
	int ret;

	swtim.cb = logic_show_main_screen;
	swtim.data = &rtc;
	swtim.period = TIM_PERIOD;

	logic_init_drivers();

	ret = swtimer_tim_register(&swtim);
	if (ret < 0) {
		pr_emerg("Error: Can't register timer: %d\n", ret);
		hang();
	}

	wh1602_control_display(&wh, LCD_ON, CURSOR_OFF, CURSOR_BLINK_OFF);
	logic.stage = STAGE_MAIN_SCREEN;
}

static void logic_handle_stage_alarm(void)
{
	int err;
	char alarm_time[BUF_LEN];
	char *flag;
	struct tm *t;

	err = ds3231_read_alarm(&rtc);
	if (err) {
		pr_emerg("Error: unable to get alarm data %d\n", err);
		hang();
	}

	t = (struct tm *)(&rtc.alarm.time);
	time2str(t, alarm_time);

	flag = (rtc.alarm.status == true) ? "Alarm ON" : "Alarm OFF";

	wh1602_clear_display(&wh);
	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, alarm_time);
	wh1602_set_line(&wh, LINE_2);
	wh1602_print_str(&wh, flag);
}

static void logic_handle_stage_alarm_toggle(void)
{
	int err;

	if (rtc.alarm.status == false)
		err = ds3231_enable_alarm(&rtc);
	else
		err = ds3231_disable_alarm(&rtc);

	if (err) {
		pr_emerg("Error: can't control alarm: %d\n", err);
		hang();
	}

	logic_handle_event(EVENT_START);
}

static void logic_handle_stage_alarm_incr_hh(void)
{
	int err;

	rtc.alarm.time.tm_hour = (rtc.alarm.time.tm_hour + 1) % 24;
	rtc.alarm.time.tm_sec = 0;

	err = ds3231_set_alarm(&rtc);
	if (err) {
		pr_emerg("Error: unable to set alarm: %d\n", err);
		hang();
	}

	logic_handle_event(EVENT_START);
}

static void logic_handle_stage_alarm_incr_mm(void)
{
	int err;

	rtc.alarm.time.tm_min = (rtc.alarm.time.tm_min + 1) % 60;
	rtc.alarm.time.tm_sec = 0;

	err = ds3231_set_alarm(&rtc);
	if (err) {
		pr_emerg("Error: unable to set alarm: %d\n", err);
		hang();
	}

	logic_handle_event(EVENT_START);
}

static void logic_handle_stage_time_set_hh(void)
{
	struct tm *t;
	int err;
	char time[BUF_LEN];

	err = ds3231_read_time(&rtc, &tm);
	if (err) {
		pr_emerg("Error: Can't read time from ds3231\n");
		hang();
	}

	t = (struct tm *)(&tm);

	time2str(t, time);

	wh1602_control_display(&wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_ON);
	wh1602_clear_display(&wh);
	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, time);
	wh1602_set_address(&wh, 0x01);
}

static void logic_handle_stage_time_incr_hh(void)
{
	int err;

	tm.tm_hour = (tm.tm_hour + 1) % 24;

	err = ds3231_set_time(&rtc, &tm);
	if (err) {
		pr_emerg("Error: Can't set hours to ds32312\n");
		hang();
	}

	logic_handle_event(EVENT_START);
}

static void logic_handle_stage_time_set_mm(void)
{
	struct tm *t;
	int err;
	char time[BUF_LEN];

	err = ds3231_read_time(&rtc, &tm);
	if (err) {
		pr_emerg("Error: Can't read time from ds3231\n");
		hang();
	}

	t = (struct tm *)(&tm);

	time2str(t, time);

	wh1602_control_display(&wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_ON);
	wh1602_clear_display(&wh);
	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, time);
	wh1602_set_address(&wh, 0x04);
}

static void logic_handle_stage_time_incr_mm(void)
{
	int err;

	tm.tm_min = (tm.tm_min + 1) % 60;

	err = ds3231_set_time(&rtc, &tm);
	if (err) {
		pr_emerg("Error: Can't set hours to ds32312\n");
		hang();
	}

	logic_handle_event(EVENT_START);
}

logic_handle_stage_func_t logic_stage_handler[STAGE_NUM] = {
	NULL,					/* STAGE_UNDEFINED */
	logic_handle_stage_init,		/* STAGE_INIT */
	logic_handle_stage_main_screen,		/* STAGE_MAIN_SCREEN */
	logic_handle_stage_main_menu,		/* STAGE_MAIN_MENU */
	logic_handle_stage_time_set_hh,		/* STAGE_TIME_SET_HH */
	logic_handle_stage_time_incr_hh,	/* STAGE_TIME_INCR_HH */
	logic_handle_stage_time_set_mm,		/* STAGE_TIME_SET_MM */
	logic_handle_stage_time_incr_mm,	/* STAGE_TIME_INCR_MM */
	logic_handle_stage_alarm,		/* STAGE_ALARM */
	logic_handle_stage_alarm_toggle,	/* STAGE_ALARM_TOGGLE */
	logic_handle_stage_alarm_incr_hh,	/* STAGE_ALARM_INCR_HH */
	logic_handle_stage_alarm_incr_mm,	/* STAGE_ALARM_INCR_MM */
};

/**
 * Process the transition for specified event.
 *
 * Run the function corresponding to current transition and set new state as
 * a current state.
 *
 * Complexity: O(1).
 *
 * @param event Event that user chose
 */
static void logic_handle_event(enum logic_event event)
{
	enum logic_stage stage = logic.stage;	/* current stage */
	enum logic_stage new_stage;
	logic_handle_stage_func_t transition_func;

	if (event > EVENT_DOWN || stage > STAGE_DATE_DECR_YY) {
		pr_emerg("Error: Wrong transition: stage = %d, "
			 "event = %d/n", stage, event);
		hang();
	}

	new_stage = logic_transitions[stage][event];

	transition_func = logic_stage_handler[new_stage];
	if (!transition_func) {
		pr_emerg("Error: Transition function doesn't exist for "
				"stage %d\n", new_stage);
		hang();
	}

	logic.stage = new_stage;
	transition_func();
}

static void logic_handle_btn(int button, bool pressed)
{
	enum logic_event btn_event = button + 1;

	if (pressed)
		logic_handle_event(btn_event);
}

void logic_start(void)
{
	logic_handle_event(EVENT_START);
}
