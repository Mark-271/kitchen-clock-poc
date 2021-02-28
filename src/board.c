#include <board.h>
#include <tools/common.h>
#include <stddef.h>

enum pull_mode {
	PULL_NO = 0,
	PULL_UP,
	PULL_DOWN
};

struct pin_mode {
	uint32_t port;
	uint16_t pins;
	uint8_t mode;
	uint8_t conf;
	enum pull_mode pull;
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
		.pull = PULL_UP,
	},
	{
		.port = KBD_GPIO_PORT,
		.pins = KBD_GPIO_R1_PIN | KBD_GPIO_R2_PIN,
		.mode = GPIO_MODE_OUTPUT_2_MHZ,
		.conf = GPIO_CNF_OUTPUT_OPENDRAIN,
	},
};

enum rcc_periph_clken clocks[] = {
	SERIAL_USART_RCC,
	SERIAL_GPIO_RCC,
	DS18B20_GPIO_RCC,
	WH1602_GPIO_RCC,
	KBD_GPIO_RCC,
	KBD_AFIO_RCC,
	KBD_TIM_RCC,
	SWTIMER_TIM_RCC,
};

/**
 * Configure which internal pull resistor to use if it's set (up or down).
 *
 * @param mode Pins to configure pull resistors for
 */
static void board_config_pull(const struct pin_mode *mode)
{
	unsigned long flags;
	uint16_t port;

	if (!mode->pull)
		return;

	enter_critical(flags);
	port = gpio_port_read(mode->port);
	switch (mode->pull) {
	case PULL_UP:
		port |= mode->pins;
		break;
	case PULL_DOWN:
		port &= ~mode->pins;
		break;
	default:
		break;
	}
	gpio_port_write(mode->port, port);
	exit_critical(flags);
}

static void board_pinmux_init(void)
{
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pins); ++i) {
		gpio_set_mode(pins[i].port, pins[i].mode, pins[i].conf,
			      pins[i].pins);
		board_config_pull(&pins[i]);
	}
}

static void board_clock_init(void)
{
	size_t i;

	rcc_clock_setup_in_hse_8mhz_out_24mhz();

	for (i = 0; i < ARRAY_SIZE(clocks); ++i)
		rcc_periph_clock_enable(clocks[i]);
}

int board_init(void)
{
	board_clock_init();
	board_pinmux_init();
	return 0;
}
