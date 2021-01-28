#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

struct kbd {
	uint32_t port;
	uint16_t l1;	/* Sampling line L1 */
	uint16_t l2;	/* Sampling line L2 */
	uint16_t r1;	/* Scan line R1 */
	uint16_t r2;	/* Scan line R2 */
};

enum kbd_btn {
	KBD_BTN_1 = 1,
	KBD_BTN_2,
	KBD_BTN_3,
	KBD_BTN_4
};

typedef void (*kbd_btn_event_t)(enum kbd_btn btn, bool pressed);

int kbd_init(struct kbd *obj, kbd_btn_event_t cb);
void kbd_exit(struct kbd *obj);

#endif /* KEYBOARD_H */
