#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

struct kb {
	uint32_t port;
	uint16_t l1_pin;	/* Sampling line 1 */
	uint16_t l2_pin;	/* Sampling line 2 */
	uint16_t r1_pin;	/* Scan line 1 */
	uint16_t r2_pin;	/* Scan line 2 */
};

int keyboard_init(struct kb *obj);
void keyboard_exit(struct kb *obj);
int keyboard_push_button(struct kb *obj);

#endif /* KEYBOARD_H */
