#ifndef DS18B20_H
#define DS18B20_H

#include <one_wire.h>
#include <stdint.h>

/* Contains parsed data from DS18B20 temperature sensor */
struct ds18b20_temp {
	uint16_t integer:12;
	uint16_t frac;
	char sign;		/* '-' or '+' */
};

struct ds18b20_temp ds18b20_read_temp(struct ow *obj);
char *ds18b20_temp2str(struct ds18b20_temp *obj, char str[]);

#endif /* DS18B20_H */
