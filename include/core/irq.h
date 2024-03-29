/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 */

#ifndef CORE_IRQ_H
#define CORE_IRQ_H

#include <tools/common.h>

enum irqreturn {
	IRQ_NONE	= 0,		/* IRQ was not from this device */
	IRQ_HANDLED	= BIT(0),	/* IRQ was handled by this device */
};

typedef enum irqreturn irqreturn_t;
typedef irqreturn_t (*irq_handler_t)(int irq, void *data);

struct irq_action {
	irq_handler_t handler;		/* interrupt handler function */
	unsigned int irq;		/* IRQ number */
	const char *name;		/* name of the device */
	void *data;			/* private user data (cookie) */
	struct irq_action *next;	/* list (shared pointer) */
};

int irq_init(void);
void irq_exit(void);
int irq_request(struct irq_action *action);
int irq_free(struct irq_action *action);

#endif /* CORE_IRQ_H */
