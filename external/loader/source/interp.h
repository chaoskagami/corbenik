#ifndef __INTERP_H
#define __INTERP_H

int execb(uint64_t tid, uint16_t ver, uint8_t *text_mem, uint32_t text_len, uint8_t *data_mem, uint32_t data_size, uint8_t *ro_mem, uint32_t ro_size);

#endif
