#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>

/* Should be initialized by the caller */
struct btn {
	uint32_t port;
	uint16_t pin;
};

int button_init(struct btn *obj);
void button_exit(struct btn *obj);
uint16_t button_poll_input(struct btn *obj);

#endif /* BUTTON_H */
