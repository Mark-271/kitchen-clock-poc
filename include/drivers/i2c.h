#ifndef DRIVERS_I2C_H
#define DRIVERS_I2C_H

#include <stdint.h>

int i2c_init(uint32_t base);
int i2c_detect_device(uint8_t addr);
int i2c_write_buf_poll(uint8_t addr, uint8_t reg, const uint8_t *buf,
		       uint16_t len);
int i2c_read_byte_pol(uint8_t addr, uint8_t reg);

#endif /* DRIVERS_I2C_H */
