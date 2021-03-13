#ifndef CORE_SYSTICK_H
#define CORE_SYSTICK_H

#include <stdint.h>

int systick_init(void);
void systick_exit(void);
uint32_t systick_get_time_ms(void);
uint32_t systick_get_time_us(void);

#endif /* CORE_SYSTICK_H */
