#ifndef ONE_WIRE_H
#define ONE_WIRE_H

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

void write_bit(uint32_t gpio_port, uint16_t gpio_pin, uint8_t bit);
uint16_t read_bit(uint32_t gpio_port, uint16_t gpio_pin);

#endif /* ONE_WIRE_H */
