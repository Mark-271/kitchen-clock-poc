#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

struct serial_params {
	uint32_t uart;
	uint32_t baud;
	uint32_t bits;
	uint32_t stopbits;
	uint32_t parity;
	uint32_t mode;
	uint32_t flow_control;
};

int _write(int fd, char *ptr, int len);
int sc_init(struct serial_params *params);
void sc_exit(void);

#endif /* SERIAL_H */
