#ifndef __INTERP_H
#define __INTERP_H

/* Loads and prepares/executes/caches a bytecode patch.
 *
 * \param filename Filename of patch to load and parse.
 * \param build_cache If zero, execute the file. Otherwise, cache the file for later.
 * \return Zero on success.
 */

int execb(const char *filename, int build_cache);

/* Low level function to actually execute the bytecode. Do not call directly.
 *
 * \param bytecode Bytecode data in memory.
 * \param len Length of bytecode in bytes.
 * \param stack Stack pointer for VM (grows up)
 * \param stack_size Size of the stack space (in bytes)
 * \param ver Version of Exefs being patched. Only used in loader (subject to change)
 * \param debug Whether to output debug information from each step of the VM to stderr
 */
int exec_bytecode(uint8_t *bytecode, uint32_t len, uint8_t* stack, uint32_t stack_size, uint16_t ver, int debug);

#endif
