#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

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
int serial_init(struct serial_params *params);
void serial_exit(void);

#endif /* DRIVERS_SERIAL_H */
