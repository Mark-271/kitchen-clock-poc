#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/cm3/cortex.h>

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

#include <common.h>
#include <delay.h>
#include <ds18b20.h>

struct tempval {
	uint16_t integer:12;
	uint16_t frac;
	char sign;		/* '-' or '+' */
};

int _write(int fd, char *ptr, int len);

/**
 * Parse temperature register from DS18B20.
 *
 * @param lsb Least significant byte of temperature register
 * @param msb Most significant byte of temperature register
 * @return Parsed value
 */
static struct tempval parse_temp(uint8_t lsb, uint8_t msb)
{
	struct tempval tv;

	tv.integer = (msb << 4) | (lsb >> 4);
	if (msb & BIT(7)) {
		tv.sign = '-';
		/*
		 * Handle negative 2's complement frac value:
		 *   1. Take first 4 bits from LSB (negative part)
		 *   2. Append missing 1111 bits (0xf0), to be able to invert
		 *      8-bit variable
		 *   3. -1 and invert (to handle 2's complement format),
		 *      accounting for implicit integer promotion rule (by
		 *      casting result to uint8_t)
		 */
		tv.frac = 625 * (uint8_t)(~(((lsb & 0xf) - 1) | 0xf0));
		/* Handle negative 2's complement integer value */
		tv.integer = ~tv.integer;
		if (tv.frac == 0)
			tv.integer++;
	} else {
		tv.sign = '+';
		tv.frac = 625 * (lsb & 0xf);
	}

	return tv;
}

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

int main(void)
{
	struct ow ow;

	clock_setup();
	gpio_setup();
	usart_setup();
	ow_init(&ow, GPIOB, GPIO9);

	printf("0x%x\n", !!(gpio_get(GPIOB, GPIO9) & BIT(9)));

	for (;;) {
		struct tempval temp;
		int8_t data[2];
		int i;

		gpio_toggle(GPIOC, GPIO8);

		mdelay(5000);

		cm_disable_interrupts();
		ow_reset(&ow);
		ow_write_byte(&ow, SKIP_ROM);
		ow_write_byte(&ow, CONVERT_T);

		cm_enable_interrupts();
		mdelay(900);
		cm_disable_interrupts();

		ow_reset(&ow);
		ow_write_byte(&ow, SKIP_ROM);
		ow_write_byte(&ow, READ_SCRATCHPAD);

		for (i = 0; i < 2; i++)
			data[i] = ow_read_byte(&ow);
		ow_reset(&ow);
		cm_enable_interrupts();

		temp = parse_temp(data[0], data[1]);
		while (temp.frac > 9)
			temp.frac /= 10;
		printf("%c%d.%d\n", temp.sign, temp.integer, temp.frac);
	}

	return 0;
}
