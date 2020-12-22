#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/cortex.h>

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

	printf("0x%x\n", (gpio_get(GPIOB, GPIO9) >> 9 & 0x01));

	int i;
	int8_t data[2];
	int16_t t = 0;

	for (;;) {
		gpio_toggle(GPIOC, GPIO8);

		cm_disable_interrupts();
		mdelay(5000);

		ow_reset(&ow);
		ow_write_byte(&ow, SKIP_ROM);
		ow_write_byte(&ow, CONVERT_T);
		mdelay(900);

		ow_reset(&ow);
		ow_write_byte(&ow, SKIP_ROM);
		ow_write_byte(&ow, READ_SCRATCHPAD);

		for (i = 0; i < 2; i++) {
			data[i] = ow_read_byte(&ow);
		}
		ow_reset(&ow);
		cm_enable_interrupts();

		t = (int16_t)((data[0] | (data[1] << 8)) / 16);
		printf("%d\n", t);
	}

	return 0;
}
