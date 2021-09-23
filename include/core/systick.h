#ifndef CORE_SYSTICK_H
#define CORE_SYSTICK_H

#include <stdint.h>

#define NSEC_PER_SEC		1e9

struct systick_time {
	uint32_t sec;
	uint32_t nsec;
};

int systick_init(void);
void systick_exit(void);
void systick_get_time (struct systick_time *t);
uint64_t systick_calc_diff(const struct systick_time *t1,
			   const struct systick_time *t2);
uint32_t systick_get_time_ms(void);
uint32_t systick_get_time_us(void);
uint32_t systick_get_time_ns(void);
uint32_t systick_calc_diff_ms(uint32_t t1, uint32_t t2);
uint32_t systick_calc_diff_ns(uint32_t t1, uint32_t t2);

#endif /* CORE_SYSTICK_H */
