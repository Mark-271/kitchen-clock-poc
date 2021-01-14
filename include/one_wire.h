#ifndef ONE_WIRE_H
#define ONE_WIRE_H

struct ow {
	uint32_t port;
	uint16_t pin;
	bool ow_flag;
};

/* 1-wire specific delay timings */
enum times {
	RESET_TIME = 500,
	PRESENCE_WAIT_TIME = 70,
	WRITE_1_TIME = 10,
	WRITE_1_PAUSE = 50,
	WRITE_0_TIME = 60,
	READ_INIT_TIME = 5,
	READ_SAMPLING_TIME = 5,
	READ_PAUSE = 50,
	SLOT_WINDOW = 5,
};

int ow_init(struct ow *ow);
void ow_exit(struct ow *ow);
int ow_reset_pulse(struct ow *ow);
void ow_write_byte(struct ow *ow, uint8_t byte);
int8_t ow_read_byte(struct ow *ow);

#endif /* ONE_WIRE_H */
