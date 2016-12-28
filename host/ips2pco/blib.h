/* Common functions for binary modifying programs. */

#ifndef __BINLIB_H
#define __BINLIB_H

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <unistd.h>

#include "util.h"

#ifdef MAX
#undef MAX
#endif
#define MAX(a,b) \
  ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

// map_file flags
#define READ_FILE  0
#define WRITE_FILE 1 // Implies read

// Hexdump feature bits
#define USE_SPACES      1
#define PRINT_OFFSET    2
#define USE_COLON       4
#define LINE_BREAKS     8
#define PIPE_SEPARATE   16
#define PIPE_OFFSET     32
#define WITH_ASCII      64
#define BYTE_A          128  // 1byte
#define BYTE_B          256  // 2byte
#define BYTE_C          512  // 4byte
#define CENTER_SPLIT    1024
#define NONPRINT_PERIOD 2048
#define NONPRINT_UNDERS 4096
#define COLORIZED       8192

// Hexdump presets.
#define SPACED_BYTES       USE_SPACES | BYTE_A
#define PRESET_XXD         USE_SPACES | BYTE_B | PRINT_OFFSET | USE_COLON | WITH_ASCII | LINE_BREAKS | NONPRINT_PERIOD
#define PRESET_HEXDUMP_C   USE_SPACES | BYTE_A | PRINT_OFFSET | PIPE_SEPARATE | WITH_ASCII | LINE_BREAKS | CENTER_SPLIT | NONPRINT_PERIOD
#define PRESET_FANCY       USE_SPACES | BYTE_A | PRINT_OFFSET | PIPE_SEPARATE | WITH_ASCII | LINE_BREAKS | CENTER_SPLIT | PIPE_OFFSET

// None of these have any meaning, but serve as documentation.
#ifdef NO_STRICT_SPECS
  #define __READ
#else
  #define __READ const
#endif

#define __WRITE
#define __WRITEREAD

// Buffer size.
#define BUFFER_SIZE 1024

// Copy file.
int copy_file(__READ char* dest, __READ char* src);

// Hexdump
int hexdump_file(__READ uint8_t *buffer, __READ uint64_t len, __READ int format);
int hexdump_manual(__READ uint64_t offset, __READ uint8_t* buffer, __READ uint64_t len, __READ int format, FILE* output);

// Unhexdump
int unhexdump_buffer(__READ uint8_t* buffer, __READ uint64_t len, __WRITE uint8_t* output);

#endif
