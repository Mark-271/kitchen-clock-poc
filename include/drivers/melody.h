#ifndef DRIVERS_MELODY_H
#define DRIVERS_MELODY_H

#include <drivers/buzzer.h>

void melody_play(struct buzz *obj);
void melody_stop(struct buzz *obj);

#endif /* DRIVERS_MELODY_H */
