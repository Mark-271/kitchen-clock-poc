#ifndef CONFIG_H
#define CONFIG_H

/* Serial console */
#define CONFIG_SERIAL_CONSOLE
#define CONFIG_SERIAL_SPEED		115200

/* Vector table size (=sizeof(vector_table) */
#define CONFIG_VTOR_SIZE		0x150

/* GPIO level transient time, usec */
#define CONFIG_GPIO_STAB_DELAY		10

#endif /* CONFIG_H */
