// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 */

#include <core/reset.h>
#include <libopencm3/stm32/rcc.h>

/* Keep it in sync with enum reset_cause */
static const char *reset_cause_names[] = {
	"Unknown",
	"Low Power Reset",
	"Window Watchdog Reset",
	"Independent Watchdog Reset",
	"Software Reset",
	"Power-On / Power-Down Reset",
	"External Pin Reset",
};

/**
 * Obtain the STM32 system reset cause.
 *
 * @return The system reset cause
 */
enum reset_cause reset_cause(void)
{
	const uint32_t csr = RCC_CSR & RCC_CSR_RESET_FLAGS;
	enum reset_cause ret;

	if (csr & RCC_CSR_LPWRRSTF)
		ret = RESET_LPWR;
	else if (csr & RCC_CSR_WWDGRSTF)
		ret = RESET_WWDG;
	else if (csr & RCC_CSR_IWDGRSTF)
		ret = RESET_IWDG;
	else if (csr & RCC_CSR_SFTRSTF)
		ret = RESET_SOFT;
	else if (csr & RCC_CSR_PORRSTF)
		ret = RESET_POR;
	else if (csr & RCC_CSR_PINRSTF)
		ret = RESET_PIN;
	else
		ret = RESET_UNKNOWN;

	return ret;
}

/**
 * Clear all reset flags.
 */
void reset_clear(void)
{
	/*
	 * Clear all the reset flags or else they will remain set during future
	 * resets until system power is fully removed.
	 */
	RCC_CSR |= RCC_CSR_RMVF;
}

/**
 * Obtain printable name string from a reset cause type.
 *
 * @param cause The previously-obtained system reset cause
 * @return A null-terminated ASCII name string describing the system reset cause
 */
const char *reset_cause_name(enum reset_cause cause)
{
	return reset_cause_names[cause];
}
