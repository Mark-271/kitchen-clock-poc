#ifndef DRIVERS_KBD_H
#define DRIVERS_KBD_H

#include <libopencm3/stm32/exti.h>
#include <libopencm3/cm3/nvic.h>
#include <stdint.h>
#include <stdbool.h>

#define KBD_SCAN_LINES		2
#define KBD_READ_LINES		2
#define KEYS			4

typedef void (*kbd_btn_event_t)(int button, bool pressed);

struct kbd_gpio {
	uint32_t port;
	uint16_t read[KBD_READ_LINES];		/* sampling lines */
	uint16_t scan[KBD_SCAN_LINES];		/* scan lines */
	int irq[KBD_READ_LINES];		/* external interrupt number */
	enum exti_trigger_type trigger;		/* exti trigger condition */
};

struct kbd {
	struct kbd_gpio gpio;	/* user data */
	kbd_btn_event_t cb;	/* callback */
	uint16_t scan_mask;	/* cached mask for scan pins */
	uint16_t read_mask;	/* cached mask for read pins */
	bool pressed[KEYS];	/* storage for button pushed/released state */
	int btn_task_id;	/* task id for task manager */
	bool scan_pending;	/* Allows external interrupts */
	int timer_id;		/* timer id for software timer */
};

int kbd_init(struct kbd *obj, const struct kbd_gpio *gpio, kbd_btn_event_t cb);
void kbd_exit(struct kbd *obj);

#endif /* DRIVERS_KBD_H */
