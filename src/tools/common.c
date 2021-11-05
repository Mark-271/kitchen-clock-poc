// SPDX-License-Identifier: GPL-3.0-or-later

#include <tools/common.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/gpio.h>

/**
 * Error handling routine.
 *
 * Hang in endless loop.
 */
void __attribute__((__noreturn__)) hang(void)
{
	for (;;)
		;
}

/**
 * Matches gpios to exti.
 *
 * @param gpio GPIO pin
 * @return User interrupt irq on success or -1 on error
 */
int gpio2irq(uint16_t gpio)
{
	if (gpio == GPIO0)
		return NVIC_EXTI0_IRQ;
	else if (gpio == GPIO1)
		return NVIC_EXTI1_IRQ;
	else if (gpio == GPIO2)
		return NVIC_EXTI2_IRQ;
	else if (gpio == GPIO3)
		return NVIC_EXTI3_IRQ;
	else if (gpio == GPIO4)
		return NVIC_EXTI4_IRQ;
	else if (gpio >= GPIO5 && gpio <= GPIO9)
		return NVIC_EXTI9_5_IRQ;
	else if (gpio >= GPIO10 && gpio <= GPIO15)
		return NVIC_EXTI15_10_IRQ;
	else
		return -1;
}
