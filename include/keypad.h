#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

struct kpd {
	uint32_t port;
	uint16_t l1_pin;	/* Sampling line 1 */
	uint16_t l2_pin;	/* Sampling line 2 */
	uint16_t r1_pin;	/* Scan line 1 */
	uint16_t r2_pin;	/* Scan line 2 */
};

enum kpd_button_code {
	KPD_BTN_1 = 1,
	KPD_BTN_2 = 2,
	KPD_BTN_3 = 3,
	KPD_BTN_4 = 4,
};

int kpd_init(struct kpd *obj);
void kpd_exit(struct kpd *obj);
int kpd_push_button(struct kpd *obj);

#endif /* KEYPAD_H */
