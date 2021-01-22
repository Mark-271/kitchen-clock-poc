#include <board.h>
#include <common.h>
#include <delay.h>
#include <ds18b20.h>
#include <one_wire.h>
#include <serial.h>
#include <wh1602.h>
#include <button.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/timer.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define SCAN_TEMPERATURE_DELAY	10000
#define LCD_GREETING_DELAY	2000

static bool timer_event_flag;
static struct ow ow = {
	.port = DS18B20_GPIO_PORT,
	.pin = DS18B20_GPIO_PIN,
	.ow_flag = false
};
static struct wh1602 wh;
static struct btn btn = {
	.port = BUTTON_GPIO_PORT,
	.pin = BUTTON_GPIO_PIN
};

static void timer_init(void)
{
	rcc_periph_clock_enable(RCC_TIM4);

	timer_set_mode(TIM4, TIM_CR1_CKD_CK_INT,
		       TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP );
	timer_set_prescaler(TIM4, 999999); /* Timer frequency is 1 MHz */
	timer_set_period(TIM4, 99999); /* Overflow occures every 100 ms */

	nvic_enable_irq(NVIC_TIM4_IRQ);
	timer_enable_irq(TIM4, TIM_DIER_UIE);

	timer_enable_counter(TIM4);
}

void tim4_isr(void)
{
	if (timer_get_flag(TIM4, TIM_SR_UIF))
			timer_clear_flag(TIM4, TIM_SR_UIF);
	timer_event_flag = true;
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
	timer_init();

	button_init(&btn);

	err = ow_init(&ow);
	if (err)
		ow.ow_flag = false;

	err = wh1602_init(&wh, &wh_gpio);
	if (err)
		hang(err);

	wh1602_set_line(&wh, LINE_1);
	wh1602_print_str(&wh, str);
	wh1602_control_display(&wh, LCD_ON, CURSOR_OFF, CURSOR_BLINK_OFF);
	mdelay(LCD_GREETING_DELAY);
	wh1602_clear_display(&wh);
}


static void __attribute__((__noreturn__)) loop(void)
{
	int c = 0 + '0';

	for (;;) {
		if (ow.ow_flag) {
			struct tempval temp;
			char buf[20];
			char *temper;

			temp = ds18b20_get_temperature(&ow);
			while (temp.frac > 9)
				temp.frac /= 10;
			temper = tempval_to_str(&temp, buf);
			wh1602_set_line(&wh, LINE_2);
			wh1602_print_str(&wh, temper);
			mdelay(SCAN_TEMPERATURE_DELAY);
		}

		if (timer_event_flag) {
			timer_event_flag = false;

			if (!button_poll_input(&btn)) {
				puts("Button pushed");
				wh1602_set_line(&wh, LINE_2);
				wh1602_write_char(&wh, c);
				c = (c == 9) ? 0 : c + 1; /* Doesn't zero out */
			}
		}
	}
}

int main(void)
{
	init();
	loop();
}
