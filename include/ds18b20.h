#ifndef DS18b20_H
#define DS18b20_H

#include <one_wire.h>
#include <stdint.h>

/* Contains parsed data from DS18B20 temperature sensor */
struct tempval {
	uint16_t integer:12;
	uint16_t frac;
	char sign;		/* '-' or '+' */
};

/* Function command set */
enum cmd {
	SKIP_ROM = 0xCC,
	CONVERT_T = 0x44,
	READ_SCRATCHPAD = 0xBE
};

struct tempval ds18b20_get_temperature(struct ow *obj);
char *tempval_to_str(struct tempval *tv, char str[]);

#endif /* DS18b20_H */
