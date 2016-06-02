#ifndef __INTERNAL_H
#define __INTERNAL_H

// These are libc builtins, so there's no need to carry an implementation here.
void *memcpy(void *dest, const void *src, size_t len);
size_t strlen(const char *string);

#endif
