#include <board.h>
#include <common.h>
#include <stddef.h>

struct pin_mode {
	uint32_t port;
	uint16_t pins;
	uint8_t mode;
	uint8_t conf;
};

static struct pin_mode pins[] = {
	{
		.port = SERIAL_GPIO_PORT,
		.pins = SERIAL_GPIO_TX_PIN,
		.mode = GPIO_MODE_OUTPUT_50_MHZ,
		.conf = GPIO_CNF_OUTPUT_ALTFN_PUSHPULL,
	},
	{
		.port = DS18B20_GPIO_PORT,
		.pins = DS18B20_GPIO_PIN,
		.mode = GPIO_MODE_OUTPUT_2_MHZ,
		.conf = GPIO_CNF_OUTPUT_OPENDRAIN,
	},
	{
		.port = WH1602_GPIO_PORT,
		.pins = WH1602_RS_PIN | WH1602_EN_PIN | WH1602_DB4_PIN |
			WH1602_DB5_PIN | WH1602_DB6_PIN | WH1602_DB7_PIN,
		.mode = GPIO_MODE_OUTPUT_2_MHZ,
		.conf = GPIO_CNF_OUTPUT_PUSHPULL,
	},
	{
		.port = KBD_GPIO_PORT,
		.pins = KBD_GPIO_L1_PIN | KBD_GPIO_L2_PIN,
		.mode = GPIO_MODE_INPUT,
		.conf = GPIO_CNF_INPUT_PULL_UPDOWN,
	},
	{
		.port = KBD_GPIO_PORT,
		.pins = KBD_GPIO_R1_PIN | KBD_GPIO_R2_PIN,
		.mode = GPIO_MODE_OUTPUT_2_MHZ,
		.conf = GPIO_CNF_OUTPUT_PUSHPULL,
	},
};

enum rcc_periph_clken clocks[] = {
	SERIAL_USART_RCC,
	SERIAL_GPIO_RCC,
	DS18B20_GPIO_RCC,
	WH1602_GPIO_RCC,
	KBD_GPIO_RCC
};

static void board_pinmux_init(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pins); ++i) {
		gpio_set_mode(pins[i].port, pins[i].mode, pins[i].conf,
			      pins[i].pins);
	}
}

static void board_clock_init(void)
{
	size_t i;

	rcc_clock_setup_in_hsi_out_48mhz();

	for (i = 0; i < ARRAY_SIZE(clocks); ++i)
		rcc_periph_clock_enable(clocks[i]);
}

int board_init(void)
{
	board_clock_init();
	board_pinmux_init();
	return 0;
}
