/* SPDX-License-Identifier: GPL-3.0-or-later */

#ifndef TOOLS_BCD_H
#define TOOLS_BCD_H

#include <stdint.h>

/**
 * Convert from binary to binary coded decimal (bcd) format.
 *
 * Implemented only for decimal two-digit numbers
 *
 * @param val Initial binary number
 * @return Encoded value
 */
static uint8_t dec2bcd(uint8_t val)
{
	return ((val / 10) << 4) | (val % 10);
}

/**
 * Convert from bcd to binary.
 *
 * @param val Encoded bcd value
 * @return Decoded binary number
 */
static uint8_t bcd2dec(uint8_t val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

#endif /* TOOLS_BCD_H */
