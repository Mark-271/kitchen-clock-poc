#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#include <serial_console.h>
#include <board.h>

#include <libopencm3/stm32/usart.h>

int sc_init(struct sc *sc)
{
	usart_set_baudrate(sc->uart, sc->baud);
	usart_set_databits(sc->uart, sc->bits);
	usart_set_stopbits(sc->uart, sc->stopbits);
	usart_set_parity(sc->uart, sc->parity);
	usart_set_mode(sc->uart, sc->mode);
	usart_set_flow_control(sc->uart, sc->flow_control);
	usart_enable(sc->uart);

	return 0;
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
		usart_send_blocking(SERIAL_USART, *ptr);
		if (*ptr == '\n') {
			usart_send_blocking(SERIAL_USART, '\r');
			usart_send_blocking(SERIAL_USART, '\n');
		}
		i++;
		ptr++;
	}

	return i;
}
