#ifndef SERIAL_CONSOLE_H
#define SERIAK_CONSOLE_H

struct sc {
	uint32_t uart;
	uint32_t baud;
	uint32_t bits;
	uint32_t stopbits;
	uint32_t parity;
	uint32_t mode;
	uint32_t flow_control;
};

int _write(int fd, char *ptr, int len);
int sc_init(struct sc *sc);

#endif /* SERIAL_CONSOLE_H */
