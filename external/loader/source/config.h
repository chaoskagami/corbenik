#pragma once

#include <stdint.h>

struct config_file {
    unsigned int config_ver;
    unsigned int firm_ver;
    uint8_t firm_console;
    uint32_t emunand_location;

    unsigned int autoboot_enabled: 1;
	unsigned int n3ds_clock: 1;
	unsigned int n3ds_l2: 1;
	unsigned int language_emu: 1;

	/* The cakes_count and cakes data
	   is excluded because loader has
       no use for it. */
} __attribute__((packed));

