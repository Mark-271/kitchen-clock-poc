#ifndef DS18b20_H
#define DS18b20_H

#define UNUSED(x) (void)x

struct ow {
	uint32_t port;
	uint16_t pin;
};

struct tempval {
	uint16_t integer:12;
	uint16_t frac;
	char sign;		/* '-' or '+' */
};

enum cmd {
	SKIP_ROM = 0xCC,
	CONVERT_T = 0x44,
	READ_SCRATCHPAD = 0xBE
};

void ow_init(struct ow *obj, uint32_t gpio_port, uint16_t gpio_pin);
void ow_exit(struct ow *obj);
struct tempval get_temperature(struct ow *obj);

#endif /* DS18b20_H */
