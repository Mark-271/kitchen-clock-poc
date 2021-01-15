#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>

#include <board.h>
#include <common.h>
#include <delay.h>
#include <serial_console.h>
#include <one_wire.h>
#include <ds18b20.h>
#include <wh1602.h>

/**
 *  Reverse the given null-terminated string in place.
 *
 *  Swap the values in the two given variables
 *  Fails when a and b refer to same memory location
 *  This works because of three basic properties of xor:
 *  x ^ 0 = x, x ^ x = 0 and x ^ y = y ^ x for all values x and y
 *
 *  @param input should be an array, whose contents are initialized to
 *  the given string constant.
 */
static void inplace_reverse(char *str)
{
	if (str) {
		char *end = str + strlen(str) - 1;

#define XOR_SWAP(a,b) do\
		{\
			a ^= b;\
			b ^= a;\
			a ^= b;\
		} while (0)

		/* Walk inwards from both ends of the string,
		 * swapping until we get to the middle
		 */
		while (str < end) {
			XOR_SWAP(*str, *end);
			str++;
			end--;
		}
#undef XOR_SWAP
	}
}

/**
 * Convert temperature data to null-terminated string.
 *
 * @param tv Contains parsed temperature register from DS18B20 @ref parse_temp()
 * @param str Array to store string literal
 * @return Pointer to string
 */
static char *tempval_to_str(struct tempval *tv, char str[])
{
	int i = 0;
	uint16_t rem;

	if (!tv->frac) {
		str[i++] = '0';
	} else {
		while (tv->frac) {
			rem = tv->frac % 10;
			str[i++] = rem + '0';
			tv->frac /= 10;
		}
	}
	str[i++] = '.';

	if (!tv->integer) {
		str[i++] = '0';
	} else {
		while (tv->integer) {
			rem = tv->integer % 10;
			str[i++] = rem + '0';
			tv->integer /= 10;
		}
	}

	str[i++] = tv->sign;
	str[i] = '\0';

	inplace_reverse(str);

	return str;
}

/* Defines behavior of the program when error occures */
static void hang(int err)
{
	printf("Error: %d\n", err);
	for (;;)
		;
}

int main(void)
{
	int err;
	unsigned long flags;

	struct sc sc = {
		.uart = SERIAL_USART,
		.baud = 115200,
		.bits = 8,
		.stopbits = USART_STOPBITS_1,
		.parity = USART_PARITY_NONE,
		.mode = USART_MODE_TX_RX,
		.flow_control = USART_FLOWCONTROL_NONE
	};
	struct ow ow = {
		.port = DS18B20_GPIO_PORT,
		.pin = DS18B20_GPIO_PIN,
		.ow_flag = true
	};
	struct wh1602 wh = {
		.port = WH1602_GPIO_PORT,
		.rs = WH1602_RS_PIN,
		.en = WH1602_EN_PIN,
		.db4 = WH1602_DB4_PIN,
		.db5 = WH1602_DB5_PIN,
		.db6 = WH1602_DB6_PIN,
		.db7 = WH1602_DB7_PIN,
	};

	board_init();
	sc_init(&sc);

	err = ow_init(&ow);
	if (err)
		ow.ow_flag = false;
	err = wh1602_init(&wh);
	if (err)
		hang(err);

	char *s = "Poc Watch";
	wh1602_set_addr_ddram(&wh, 0x0);
	wh1602_print_str(&wh, s);
	wh1602_display_control(&wh, DISPLAY_ON, OFF, OFF);

	enter_critical(flags);
	mdelay(2000);
	exit_critical(flags);
	wh1602_display_clear(&wh);

	for (;;) {
		if (ow.ow_flag) {
			struct tempval temp;
			char buf[20];

			temp = ds18b20_get_temperature(&ow);
			while (temp.frac > 9)
				temp.frac /= 10;
			s = tempval_to_str(&temp, buf);
			wh1602_print_str(&wh, s);
			wh1602_return_home(&wh);

			enter_critical(flags);
			mdelay(10000);
			exit_critical(flags);
		}
	}

	return 0;
}
