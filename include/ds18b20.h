#ifndef DS18b20_H
#define DS18b20_H

#include <one_wire.h>

struct tempval {
	uint16_t integer:12;
	uint16_t frac;
	char sign;		/* '-' or '+' */
};

enum cmd {
	SKIP_ROM = 0xCC,
	CONVERT_T = 0x44,
	READ_SCRATCHPAD = 0xBE
};

struct tempval get_temperature(struct ow *obj);

#endif /* DS18b20_H */
