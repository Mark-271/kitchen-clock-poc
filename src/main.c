#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <delay.h>
#include <ds18b20.h>

int _write(int fd, char *ptr, int len);

static void clock_setup(void)
{
	rcc_clock_setup_in_hsi_out_48mhz();

	rcc_periph_clock_enable(RCC_USART1);
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOC);
	rcc_periph_clock_enable(RCC_GPIOB);
}

static void gpio_setup(void)
{
	/* USART1 */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
	/* LED */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, GPIO8);
	/* Temperature sensor */
	gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_OPENDRAIN, GPIO9);

}

static void usart_setup(void)
{
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	usart_enable(USART1);
}

int _write(int fd, char *ptr, int len)
{
	int i = 0;

	/* only works for stdout */
	if (fd != 1) {
		errno = EIO;
		return -1;
	}

	while (*ptr && (i < len)) {
		usart_send_blocking(USART1, *ptr);
		if (*ptr == '\n') {
			usart_send_blocking(USART1, '\r');
			usart_send_blocking(USART1, '\n');
		}
		i++;
		ptr++;
	}

	return i;
}

struct ow ow;

int main(void)
{
	clock_setup();
	gpio_setup();
	usart_setup();
	ow_init(&ow, GPIOB, GPIO9);

	printf("HELLO\n");

	gpio_set(GPIOB, GPIO9);
	printf("0x%x\n", (gpio_get(GPIOB, GPIO9) >> 9 & 0x01));

	int ret = 0;
	ret = ow_reset(&ow);
	printf("0x%x\n", ret >> 9 & 0x01);

	printf("0x%x\n", (gpio_get(GPIOB, GPIO9) >> 9 & 0x1));

	for (;;) {
		gpio_toggle(GPIOC, GPIO8);
		mdelay(10000);
/*
		ow_reset(&ow);
		ow_write_byte(&ow, 0xCC);
		ow_write_byte(&ow, 0x44);
		mdelay(760);

		ow_reset(&ow);
		ow_write_byte(&ow, 0xCC);
		ow_write_byte(&ow, 0xBE);

		for (i = 0; i < 8; i++) {
			data[i] = ow_read_byte(&ow);
		}
		ow_reset(&ow);
		t = (int16_t)(data[0] | (data[1] << 8));

		printf("%d\n", t);
*/
	}

	return 0;
}
