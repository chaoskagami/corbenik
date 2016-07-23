#ifndef __INTERP_H
#define __INTERP_H

int execb(char *filename, int build_cache);
int exec_bytecode(uint8_t *bytecode, uint32_t len, uint8_t* stack, uint32_t stack_size, uint16_t ver, int debug);

#endif
