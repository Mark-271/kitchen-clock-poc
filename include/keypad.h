#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>
#include <stdbool.h>

struct kpd {
	uint32_t port;
	uint16_t l1_pin;	/* Sampling line 1 */
	uint16_t l2_pin;	/* Sampling line 2 */
	uint16_t r1_pin;	/* Scan line 1 */
	uint16_t r2_pin;	/* Scan line 2 */
};

enum kpd_btn {
	KPD_BTN_1 = 1,
	KPD_BTN_2,
	KPD_BTN_3,
	KPD_BTN_4
};

typedef void (*kpd_btn_event_t)(enum kpd_btn btn, bool pressed);

int kpd_init(struct kpd *obj, kpd_btn_event cb);
void kpd_exit(struct kpd *obj);

#endif /* KEYPAD_H */
