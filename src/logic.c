// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Mark Sungurov <mark.sungurov@gmail.com>
 */

/* TODO: Simplify state machine (use switch, Luke!) */
/* TODO: Put all global variables into main structure (logic) */
/* TODO: Get rid of all global variables */

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
#include <drivers/wh1602.h>
#include <tools/common.h>
#include <tools/melody.h>
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
#define TM_DEFAULT_YEAR		(EPOCH_YEAR - TM_START_YEAR)

typedef void (*logic_handle_stage_func_t)(void);

/* Keep 0 as undefined state */
enum logic_stage {
	STAGE_UNDEFINED = 0,
	STAGE_INIT,
	STAGE_MAIN_SCREEN,
	STAGE_MAIN_MENU,
	STAGE_ALARM,
	STAGE_ADJUSTMENT,
	STAGE_SET_HH,
	STAGE_SET_MM,
	STAGE_SET_WDAY,
	STAGE_SET_MON,
	STAGE_SET_MDAY,
	STAGE_SET_YEAR,
};

enum logic_event {
	EVENT_LEFT,	/* left button */
	EVENT_RIGHT,	/* right button */
	EVENT_UP,	/* up button */
	EVENT_DOWN,	/* down button */
};

struct rtc_data {
	char temper[BUF_LEN];
	char date[BUF_LEN];
	char time[BUF_LEN];
	char ctemper[BUF_LEN];
	char cdate[BUF_LEN];
	char ctime[BUF_LEN];
};

struct logic {
	enum logic_stage stage; /* current state of FSM */
	bool ds18b20_presence_flag;
	bool ds3231_presence_flag;
	struct rtc_data data;
	struct buzz buzz;
	struct ds18b20 ts;
	struct ds3231 rtc;
	struct kbd kbd;
	struct rtc_time tm;
	struct swtimer_sw_tim swtim;		/* software timer object */
	struct wh1602 wh;
};

static void logic_handle_btn(int btn, bool pressed);
static void logic_alarm_cb(void);

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


static struct logic logic;

/* Initialize peripheral drivers */
static void logic_init_drivers(void)
{
	int err;

	logic.ts.port = DS18B20_GPIO_PORT;
	logic.ts.pin = DS18B20_GPIO_PIN;

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

	err = kbd_init(&logic.kbd, &kbd_gpio, logic_handle_btn);
	if (err) {
		pr_emerg("Error: Can't initialize kbd: %d\n", err);
		hang();
	}

	err = ds18b20_init(&logic.ts);
	if (err)
		pr_warn("Warning: Can't initialize ds18b20: %d\n", err);
	logic.ds18b20_presence_flag = !err;

	err = wh1602_init(&logic.wh, &wh_gpio);
	if (err) {
		pr_emerg("Error: Can't initialize wh1602: %d\n", err);
		hang();
	}

	err = ds3231_init(&logic.rtc, &device, EPOCH_YEAR, logic_alarm_cb);
	if (err)
		pr_warn("Warning: Can't initialize ds3231: %d\n", err);
	logic.ds3231_presence_flag = !err;

	buzz_init(&logic.buzz, BUZZ_GPIO_PORT, BUZZ_GPIO_PIN);
}

static char *logic_read_temper(void *data)
{
	struct ds18b20 *obj = (struct ds18b20 *)(data);
	char buf[BUF_LEN];

	obj->temp = ds18b20_read_temp(&logic.ts);

	while (obj->temp.frac > 9)
		obj->temp.frac /= 10;

	return ds18b20_temp2str(&obj->temp, buf);
}

static void logic_display_data(struct logic *obj)
{
	wh1602_clear_display(&obj->wh);

	wh1602_set_line(&obj->wh, LINE_1);
	wh1602_print_str(&obj->wh, obj->data.time);

	wh1602_set_line(&obj->wh, LINE_2);
	wh1602_print_str(&obj->wh, obj->data.date);

	wh1602_set_address(&obj->wh, TEMPER_DISPLAY_ADDR);
	wh1602_write_char(&obj->wh, 't');
	wh1602_print_str(&obj->wh, obj->data.temper);

	if (obj->rtc.alarm.status) {
		wh1602_set_address(&obj->wh, ALARM_SYMBOL_POS);
		wh1602_write_char(&obj->wh, ALARM_INDICATOR);
	}
}

static void logic_display_cdata(struct logic *obj)
{
	wh1602_clear_display(&obj->wh);

	wh1602_set_line(&obj->wh, LINE_1);
	wh1602_print_str(&obj->wh, obj->data.ctime);

	wh1602_set_line(&obj->wh, LINE_2);
	wh1602_print_str(&obj->wh, obj->data.cdate);

	wh1602_set_address(&obj->wh, TEMPER_DISPLAY_ADDR);
	wh1602_write_char(&obj->wh, 't');
	wh1602_print_str(&obj->wh, obj->data.ctemper);

	if (obj->rtc.alarm.status) {
		wh1602_set_address(&obj->wh, ALARM_SYMBOL_POS);
		wh1602_write_char(&obj->wh, ALARM_INDICATOR);
	}
}

/* Callback to register inside swtimer */
static void logic_show_main_screen(void *data)
{
	int err;
	struct tm *t;

	UNUSED(data);

	if (logic.ds3231_presence_flag) {
		err = ds3231_read_time(&logic.rtc, &logic.tm);
		if (err) {
			pr_emerg("Error: Can't read time: %d\n", err);
			hang();
		}

		t = (struct tm *)(&logic.tm);
		date2str(t, logic.data.date);
		time2str(t, logic.data.time);
	} else {
		strcpy(logic.data.date, "00 000 0000");
		strcpy(logic.data.time, "00 00");
	}

	if (logic.ds18b20_presence_flag)
		strcpy(logic.data.temper, logic_read_temper(&logic.ts));
	else
		strcpy(logic.data.temper, "xx");

	if (strcmp(logic.data.temper, logic.data.ctemper) ||
	    strcmp(logic.data.time, logic.data.ctime)) {
		logic_display_data(&logic);
		strcpy(logic.data.ctime, logic.data.time);
		strcpy(logic.data.cdate, logic.data.date);
		strcpy(logic.data.ctemper, logic.data.temper);
	}
}

static void logic_handle_stage_main_screen(void)
{
	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_OFF, CURSOR_BLINK_OFF);
	swtimer_tim_start(logic.swtim.id);

	if (strcmp(logic.data.temper, logic.data.ctemper) ||
	    strcmp(logic.data.time, logic.data.ctime))
		logic_display_data(&logic);
	else
		logic_display_cdata(&logic);
}

static void logic_handle_stage_main_menu(void)
{
	size_t i;

	swtimer_tim_stop(logic.swtim.id);
	wh1602_clear_display(&logic.wh);

	for (i = 0; i < MENU_NUM; i++) {
		wh1602_set_address(&logic.wh, menu_addr[i]);
		wh1602_print_str(&logic.wh, menu_msg[i]);
	}
}

static void logic_handle_stage_init(void)
{
	int ret;

	logic.swtim.cb = logic_show_main_screen;
	logic.swtim.data = &logic.rtc;
	logic.swtim.period = TIM_PERIOD;

	logic_init_drivers();

	ret = swtimer_tim_register(&logic.swtim);
	if (ret < 0) {
		pr_emerg("Error: Can't register timer: %d\n", ret);
		hang();
	}

	if (logic.ds3231_presence_flag) {
		/* Year count should start from beginning of the epoch */
		logic.tm.tm_year = TM_DEFAULT_YEAR;
		logic.rtc.alarm.time.tm_year = TM_DEFAULT_YEAR;

		ret = ds3231_set_time(&logic.rtc, &logic.tm);
		if (ret != 0) {
			pr_emerg("Error: Unable to set year inside ds3231"
					"timekeeping register\n");
			hang();
		}
	}

	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_OFF, CURSOR_BLINK_OFF);
	logic.stage = STAGE_MAIN_SCREEN;
}

static void logic_handle_stage_alarm(void)
{
	char alarm_time[BUF_LEN];
	char *flag;
	struct tm *t;

	/* HACK: Brute kicking user back to main menu to disallow RTC ops */
	if (!logic.ds3231_presence_flag) {
		logic.stage = STAGE_MAIN_MENU;
		return;
	}

	t = (struct tm *)(&logic.rtc.alarm.time);
	time2str(t, alarm_time);

	flag = (logic.rtc.alarm.status == true) ? "Alarm ON" : "Alarm OFF";

	wh1602_clear_display(&logic.wh);
	wh1602_set_line(&logic.wh, LINE_1);
	wh1602_print_str(&logic.wh, alarm_time);
	wh1602_set_line(&logic.wh, LINE_2);
	wh1602_print_str(&logic.wh, flag);
}

static void logic_config_alarm(void)
{
	int err;
	bool enabled = logic.rtc.alarm.status;

	if (enabled == false) {
		err = ds3231_set_alarm(&logic.rtc);
		if (err)
			goto error;
	}

	err = ds3231_toggle_alarm(&logic.rtc, !enabled);
	if (err)
		goto error;

	return;

error:
	pr_emerg("Error: Can't control alarm: %d\n", err);
	hang();
}

static void logic_incr_alarm_hh(void)
{
	struct rtc_time *t = &logic.rtc.alarm.time;

	t->tm_hour = (t->tm_hour + 1) % 24;
	t->tm_sec = 0;
}

static void logic_incr_alarm_mm(void)
{
	struct rtc_time *t = &logic.rtc.alarm.time;

	t->tm_min = (t->tm_min + 1) % 60;
	t->tm_sec = 0;
}

static void logic_show_adjustment_screen(void)
{
	struct tm *t;
	char time[BUF_LEN];
	char date[BUF_LEN];

	t = (struct tm *)(&logic.tm);

	time2str(t, time);
	date2str(t, date);

	wh1602_clear_display(&logic.wh);
	wh1602_set_line(&logic.wh, LINE_1);
	wh1602_print_str(&logic.wh, time);
	wh1602_set_line(&logic.wh, LINE_2);
	wh1602_print_str(&logic.wh, date);
}

static void logic_handle_stage_adjustment(void)
{
	int err;

	if (!logic.ds3231_presence_flag) {
		logic.stage = STAGE_MAIN_MENU;
		return;
	}

	err = ds3231_read_time(&logic.rtc, &logic.tm);
	if (err) {
		pr_emerg("Error: Can't read time: %d\n", err);
		hang();
	}

	logic_show_adjustment_screen();
	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_ON);
	wh1602_set_address(&logic.wh, 0x0f);
}

static void logic_set_new_time(void)
{
	int err;
	struct tm *t;

	err = ds3231_set_time(&logic.rtc, &logic.tm);
	if (err) {
		pr_err("Error: Can't set time: %d\n", err);
		hang();
	}

		t = (struct tm *)(&logic.tm);
		date2str(t, logic.data.date);
		time2str(t, logic.data.time);
}

static void logic_handle_stage_set_hh(void)
{
	logic.tm.tm_hour = (logic.tm.tm_hour + 1) % 24;

	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_OFF);
	logic_show_adjustment_screen();
	wh1602_set_address(&logic.wh, 0x01);
}

static void logic_handle_stage_set_mm(void)
{
	logic.tm.tm_min = (logic.tm.tm_min + 1) % 60;

	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_OFF);
	logic_show_adjustment_screen();
	wh1602_set_address(&logic.wh, 0x04);
}

static void logic_handle_stage_set_wday(void)
{
	logic.tm.tm_wday = (logic.tm.tm_wday + 1) % 7;

	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_OFF);
	logic_show_adjustment_screen();
	wh1602_set_address(&logic.wh, 0x40);
}

static void logic_handle_stage_set_mon(void)
{
	logic.tm.tm_mon = (logic.tm.tm_mon + 1) % 12;
	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_OFF);
	logic_show_adjustment_screen();
	wh1602_set_address(&logic.wh, 0x47);
}

static void logic_handle_stage_set_mday(void)
{
	logic.tm.tm_mday = (logic.tm.tm_mday + 1) % 32;
	if (logic.tm.tm_mday == 0)
		logic.tm.tm_mday++;

	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_OFF);
	logic_show_adjustment_screen();
	wh1602_set_address(&logic.wh, 0x44);
}

static void logic_handle_stage_set_year(void)
{
	logic.tm.tm_year++;
	if (logic.tm.tm_year > TM_DEFAULT_YEAR + 10)
		logic.tm.tm_year = TM_DEFAULT_YEAR;

	wh1602_control_display(&logic.wh, LCD_ON, CURSOR_ON, CURSOR_BLINK_OFF);
	logic_show_adjustment_screen();
	wh1602_set_address(&logic.wh, 0x4b);
}

static void logic_alarm_cb(void)
{
	return;
}

static void logic_handle_stage(enum logic_stage stage)
{
	switch (stage) {
	case STAGE_UNDEFINED:
		logic_handle_stage(STAGE_INIT);
		break;
	case STAGE_INIT:
		logic_handle_stage_init();
		break;
	case STAGE_MAIN_SCREEN:
		logic_handle_stage_main_screen();
		break;
	case STAGE_MAIN_MENU:
		logic_handle_stage_main_menu();
		break;
	case STAGE_ALARM:
		logic_handle_stage_alarm();
		break;
	case STAGE_ADJUSTMENT:
		logic_handle_stage_adjustment();
		break;
	case STAGE_SET_HH:
		logic_handle_stage_set_hh();
		break;
	case STAGE_SET_MM:
		logic_handle_stage_set_mm();
		break;
	case STAGE_SET_WDAY:
		logic_handle_stage_set_wday();
		break;
	case STAGE_SET_MON:
		logic_handle_stage_set_mon();
		break;
	case STAGE_SET_MDAY:
		logic_handle_stage_set_mday();
		break;
	case STAGE_SET_YEAR:
		logic_handle_stage_set_year();
		break;
	default:
		pr_warn("Stage doesn't exist\n");
	}
}

static void logic_handle_key_press(enum logic_event event)
{
	enum logic_stage stage = logic.stage;	/* current stage */
	enum logic_stage new_stage = STAGE_UNDEFINED;

	switch (event) {
	case EVENT_LEFT:
		if (stage == STAGE_MAIN_SCREEN) {
			logic_handle_stage(STAGE_MAIN_MENU);
			new_stage = STAGE_MAIN_MENU;
		} else if (stage == STAGE_MAIN_MENU ||
			   stage == STAGE_ALARM) {
			logic_handle_stage(STAGE_MAIN_SCREEN);
			new_stage = STAGE_MAIN_SCREEN;
		} else {
			logic_set_new_time();
			logic_handle_stage(STAGE_MAIN_SCREEN);
			new_stage = STAGE_MAIN_SCREEN;
		}
		break;
	case EVENT_RIGHT:
		if (stage == STAGE_MAIN_SCREEN) {
			logic_handle_stage(STAGE_MAIN_MENU);
			new_stage = STAGE_MAIN_MENU;
		} else if (stage == STAGE_ALARM) {
			logic_config_alarm();
			logic_handle_stage(STAGE_ALARM);
			new_stage = STAGE_ALARM;
		} else if (stage == STAGE_SET_HH) {
			new_stage = STAGE_SET_MM;
		} else if (stage == STAGE_SET_MM) {
			new_stage = STAGE_SET_WDAY;
		} else if (stage == STAGE_SET_WDAY) {
			new_stage = STAGE_SET_MON;
		} else if (stage == STAGE_SET_MON) {
			new_stage = STAGE_SET_MDAY;
		} else if (stage == STAGE_SET_MDAY) {
			new_stage = STAGE_SET_YEAR;
		} else if (stage == STAGE_SET_YEAR) {
			new_stage = STAGE_SET_HH;
		}
		break;
	case EVENT_UP:
		if (stage == STAGE_MAIN_SCREEN) {
			logic_handle_stage(STAGE_MAIN_MENU);
			new_stage = STAGE_MAIN_MENU;
		} else if (stage == STAGE_MAIN_MENU) {
			logic_handle_stage(STAGE_ALARM);
			new_stage = STAGE_ALARM;
		} else if (stage == STAGE_ALARM) {
			logic_incr_alarm_hh();
			logic_handle_stage(STAGE_ALARM);
			new_stage = STAGE_ALARM;
		} else if (stage == STAGE_SET_HH) {
			logic_handle_stage(STAGE_SET_HH);
			new_stage = STAGE_SET_HH;
		} else if (stage == STAGE_SET_MM) {
			logic_handle_stage(STAGE_SET_MM);
			new_stage = STAGE_SET_MM;
		} else if (stage == STAGE_SET_WDAY) {
			logic_handle_stage(STAGE_SET_WDAY);
			new_stage = STAGE_SET_WDAY;
		} else if (stage == STAGE_SET_MON) {
			logic_handle_stage(STAGE_SET_MON);
			new_stage = STAGE_SET_MON;
		} else if (stage == STAGE_SET_MDAY) {
			logic_handle_stage(STAGE_SET_MDAY);
			new_stage = STAGE_SET_MDAY;
		} else if (stage == STAGE_SET_YEAR) {
			logic_handle_stage(STAGE_SET_YEAR);
			new_stage = STAGE_SET_YEAR;
		}
		break;
	case EVENT_DOWN:
		if (stage == STAGE_MAIN_SCREEN) {
			logic_handle_stage(STAGE_MAIN_MENU);
			new_stage = STAGE_MAIN_MENU;
		} else if (stage == STAGE_ALARM) {
			logic_incr_alarm_mm();
			logic_handle_stage(STAGE_ALARM);
			new_stage = STAGE_ALARM;
		} else if (stage == STAGE_MAIN_MENU) {
			logic_handle_stage(STAGE_ADJUSTMENT);
			new_stage = STAGE_SET_HH;
		}
		break;
	}

	logic.stage = new_stage;
}

static void logic_handle_btn(int button, bool pressed)
{
	enum logic_event event = button;

	if (pressed)
		logic_handle_key_press(event);
}

void logic_start(void)
{
	logic_handle_stage(STAGE_INIT);
}
