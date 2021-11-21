/* SPDX-License-Identifier: GPL-3.0-or-later */
/*
 * Author: Sam Protsenko <joe.skb7@gmail.com>
 *         Mark Sungurov <mark.sungurov@gmail.com>
 */

#ifndef BOARD_H
#define BOARD_H

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/stm32/usart.h>

/* Serial console */
#define SERIAL_GPIO_PORT	GPIOA
#define SERIAL_GPIO_TX_PIN	GPIO_USART1_TX
#define SERIAL_USART		USART1
#define SERIAL_USART_RCC	RCC_USART1
#define SERIAL_GPIO_RCC		RCC_GPIOA

/* Temperature sensor */
#define DS18B20_GPIO_RCC	RCC_GPIOD
#define DS18B20_GPIO_PORT	GPIOD
#define DS18B20_GPIO_PIN	GPIO2

/* LCD display */
#define WH1602_GPIO_RCC		RCC_GPIOC
#define WH1602_GPIO_PORT	GPIOC
#define WH1602_RS_PIN		GPIO4
#define WH1602_EN_PIN		GPIO5
#define WH1602_DB4_PIN		GPIO0
#define WH1602_DB5_PIN		GPIO1
#define WH1602_DB6_PIN		GPIO2
#define WH1602_DB7_PIN		GPIO3

/* Matrix keypad 2x2 */
#define KBD_AFIO_RCC		RCC_AFIO
#define KBD_GPIO_RCC		RCC_GPIOA
#define KBD_GPIO_PORT		GPIOA
#define KBD_GPIO_L1_PIN		GPIO1
#define KBD_GPIO_L2_PIN		GPIO2
#define KBD_GPIO_R1_PIN		GPIO3
#define KBD_GPIO_R2_PIN		GPIO4
#define KBD_EXTI_TRIGGER	EXTI_TRIGGER_BOTH

/* General purpose timer */
#define SWTIMER_TIM_RCC		RCC_TIM2
#define SWTIMER_TIM_BASE	TIM2
#define SWTIMER_TIM_IRQ		NVIC_TIM2_IRQ
#define SWTIMER_TIM_RST		RST_TIM2
#define SWTIMER_TIM_ARR_VAL	19999
#define SWTIMER_TIM_PSC_VAL	5
#define SWTIMER_TIM_DBGMCU	DBGMCU_CR_TIM2_STOP

/* DS3231 RTC */
#define DS3231_DEVICE_ADDR	0x68
#define DS3231_AFIO_RCC		RCC_AFIO
#define DS3231_GPIO_RCC		RCC_GPIOB
#define DS3231_I2C_RCC		RCC_I2C2
#define DS3231_I2C_BASE		I2C2
#define DS3231_I2C_GPIO_PORT	GPIOB
#define DS3231_I2C_SCL_PIN	GPIO10
#define DS3231_I2C_SDA_PIN	GPIO11
#define DS3231_ALARM_PIN	GPIO12
#define DS3231_EXTI_TRIGGER	EXTI_TRIGGER_FALLING
#define DS3231_EXTI_IRQ		NVIC_EXTI15_10_IRQ

/* Buzzer */
#define BUZZ_GPIO_RCC		RCC_GPIOC
#define BUZZ_GPIO_PORT		GPIOC
#define BUZZ_GPIO_PIN		GPIO12

int board_init(void);

#endif /* BOARD_H */
