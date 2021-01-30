#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>
#include <stdbool.h>

#define KBD_NOPRESSED		0
#define KBD_BTN_1		1
#define KBD_BTN_2		2
#define KBD_BTN_3		3
#define KBD_BTN_4		4

typedef void (*kbd_btn_event_t)(int button, bool pressed);

struct kbd_gpio {
	uint32_t port;
	uint16_t l1;		/* sampling line L1 */
	uint16_t l2;		/* sampling line L2 */
	uint16_t r1;		/* scan line R1 */
	uint16_t r2;		/* scan line R2 */
};

struct kbd {
	struct kbd_gpio gpio;	/* user data */
	kbd_btn_event_t cb;	/* callback */
	uint16_t lookup[5];	/* mapping: btn --> GPIO port value */
	int btn;		/* button code */
};

int kbd_init(struct kbd *obj, struct kbd_gpio *gpio, kbd_btn_event_t cb);
void kbd_exit(struct kbd *obj);

#endif /* KEYBOARD_H */
