#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

struct btn {
	uint32_t port;
	uint16_t pin;
	uint16_t state:1;
};

enum button_base_state {
	LOW = 0,
	HIGH
};

int button_init(struct btn *obj, uint16_t base_state);
void button_exit(struct btn *obj);
uint16_t button_poll_input(struct btn *obj);

#endif /* BUTTON_H */
