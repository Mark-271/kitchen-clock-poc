#ifndef BOARD_H
#define BOARD_H

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
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

int board_init(void);

#endif /* BOARD_H */
