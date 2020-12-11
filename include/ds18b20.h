#ifndef DS18b20_H
#define DS18b20_H

struct ow {
	uint32_t port;
	uint16_t pin;
};

void ow_init(struct ow *obj, uint32_t gpio_port, uint16_t gpio_pin);
void ow_exit(struct ow *obj);

#endif /* DS18b20_H */
