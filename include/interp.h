#ifndef __INTERP_H
#define __INTERP_H

uint8_t *execb(char *filename, int cache);
int exec_bytecode(uint8_t *bytecode, uint32_t len, int debug);

#endif
