#ifndef BOARD_H
#define BOARD_H

#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/gpio.h>
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

int board_init(void);

#endif /* BOARD_H */
