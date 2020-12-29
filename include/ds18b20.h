#ifndef DS18b20_H
#define DS18b20_H

#define UNUSED(x) (void)x

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
int8_t ow_read_byte(struct ow *obj);

#endif /* DS18b20_H */
