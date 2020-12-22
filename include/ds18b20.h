#ifndef DS18b20_H
#define DS18b20_H

#define UNUSED(x) (void)x

/* 1-wire specific delay timings */
#define RESET_TIME		500
#define PRESENCE_WAIT_TIME	70
#define WRITE_1_TIME		10
#define WRITE_1_PAUSE		50
#define WRITE_0_TIME		60
#define READ_INIT_TIME		5
#define READ_SAMPLING_TIME	5
#define READ_PAUSE		50
#define SLOT_WINDOW		5

struct ow {
	uint32_t port;
	uint16_t pin;
};

enum cmd {
	SKIP_ROM = 0xCC,
	CONVERT_T = 0x44,
	READ_SCRATCHPAD = 0xBE
};

void ow_init(struct ow *obj, uint32_t gpio_port, uint16_t gpio_pin);
void ow_exit(struct ow *obj);
int ow_reset(struct ow *obj);
void ow_write_byte(struct ow *obj, uint8_t data);

#endif /* DS18b20_H */
