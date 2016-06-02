#ifndef __STD_TYPES_H
#define __STD_TYPES_H

#include <stdint.h>
#include <stdlib.h>

#define CFG_BOOTENV *(volatile uint32_t *)0x10010000
#define HID ~*(volatile uint32_t *)0x10146000
#define PDN_MPCORE_CFG *(uint8_t *)0x10140FFC

#endif
