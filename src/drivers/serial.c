// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file
 *
 * Serial console driver.
 *
 * It's a singleton object, because it can't exist in multiple instances anyway,
 * because of _write() "syscall" implementation.
 */

#ifdef CONFIG_SERIAL_CONSOLE

#include <drivers/serial.h>
#include <tools/common.h>
#include <libopencm3/stm32/usart.h>
#include <stdio.h>
#include <errno.h>

static uint32_t serial_usart; /* singleton object */

/* Forward declaration of "syscalls", to make GCC happy */
int _write(int fd, char *ptr, int len);
void *_sbrk(intptr_t increment);

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

/* write() syscall implementation for newlib */
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

#ifdef CONFIG_SERIAL_SBRK_TRAP
/**
 * sbrk() syscall implementation for newlib when serial console is disabled.
 *
 * Nobody should call _sbrk() (e.g. via printf()) when serial console is
 * disabled. Let's hang if somebody does, so we can catch that rascal!
 */
void *_sbrk(intptr_t increment)
{
	UNUSED(increment);
	hang();
	return (void *)-1;
}
#endif /* CONFIG_SERIAL_SBRK_TRAP */

#endif /* CONFIG_SERIAL_CONSOLE */
