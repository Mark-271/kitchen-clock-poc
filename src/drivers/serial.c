/**
 * @file
 *
 * Serial console driver.
 *
 * It's a singleton object, because it can't exist in multiple instances anyway,
 * because of _write() "syscall" implementation.
 */

#include <drivers/serial.h>
#include <libopencm3/stm32/usart.h>
#include <stdio.h>
#include <errno.h>

static uint32_t serial_usart; /* singleton object */

int serial_init(struct serial_params *params)
{
	serial_usart = params->uart;

	usart_set_baudrate(serial_usart, params->baud);
	usart_set_databits(serial_usart, params->bits);
	usart_set_stopbits(serial_usart, params->stopbits);
	usart_set_parity(serial_usart, params->parity);
	usart_set_mode(serial_usart, params->mode);
	usart_set_flow_control(serial_usart, params->flow_control);
	usart_enable(serial_usart);

	return 0;
}

void serial_exit(void)
{
	usart_disable(serial_usart);
	serial_usart = 0;
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
		usart_send_blocking(serial_usart, *ptr);
		if (*ptr == '\n') {
			usart_send_blocking(serial_usart, '\r');
			usart_send_blocking(serial_usart, '\n');
		}
		i++;
		ptr++;
	}

	return i;
}
