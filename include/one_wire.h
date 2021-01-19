#ifndef ONE_WIRE_H
#define ONE_WIRE_H

#include <stdint.h>
#include <stdbool.h>

struct ow {
	uint32_t port;
	uint16_t pin;
	bool ow_flag;
};

/* 1-wire specific delay timings */
enum times {
	PRESENCE_WAIT_TIME = 70,
	READ_INIT_TIME = 5,
	READ_PAUSE = 50,
	READ_SAMPLING_TIME = 5,
	RESET_TIME = 500,
	SLOT_WINDOW = 5,
	WRITE_0_TIME = 60,
	WRITE_1_PAUSE = 50,
	WRITE_1_TIME = 10,
};

int ow_init(struct ow *obj);
void ow_exit(struct ow *obj);
int ow_reset_pulse(struct ow *obj);
void ow_write_byte(struct ow *obj, uint8_t byte);
int8_t ow_read_byte(struct ow *obj);

#endif /* ONE_WIRE_H */
