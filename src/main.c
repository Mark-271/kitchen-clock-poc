#include <errno.h>
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
#include <one_wire.h>
#include <ds18b20.h>
#include <wh1602.h>

int _write(int fd, char *ptr, int len);

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
	struct ow ow = {
		.port = DS18B20_GPIO_PORT,
		.pin = DS18B20_GPIO_PIN
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
	usart_setup();

	err = ow_init(&ow);
	// XXX: "ow_init error check" issue
#if 0
	if (err)
		hang(err);
#endif
	err = wh1602_init(&wh);
	if (err)
		hang(err);

	char *s = "I love you!";
	wh1602_set_addr_ddram(&wh, 0x0);
	wh1602_print_str(&wh, s);
	mdelay(10000);
	wh1602_erase_screen(&wh);

	for (;;) {
#if 0
		struct tempval temp;
		char buf[20];

		gpio_toggle(GPIOC, GPIO8);

		mdelay(5000);

		temp = ds18b20_get_temperature(&ow);
		while (temp.frac > 9)
			temp.frac /= 10;
		puts(tempval_to_str(&temp, buf));
#endif
	}

	return 0;
}
