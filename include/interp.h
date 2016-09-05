#ifndef __INTERP_H
#define __INTERP_H

/* Loads and caches a bytecode patch.
 *
 * \param filename Filename of patch to load and cache
 * \return Zero on success.
 */

int cache_patch(const char *filename);

/* Loads and executes cached bytecode.
 *
 * \param tid Title ID to patch.
 * \param firm Firmware structure.
 * \return Zero on success.
 */

int execb(uint64_t tid, firm_h *firm);

/* Low level function to actually execute the bytecode. Do not call directly.
 *
 * \param bytecode Bytecode data in memory.
 * \param len Length of bytecode in bytes.
 * \param stack Stack pointer for VM (grows up)
 * \param stack_size Size of the stack space (in bytes)
 * \param ver Version of Exefs being patched. Only used in loader (subject to change)
 * \param debug Whether to output debug information from each step of the VM to stderr
 */
int exec_bytecode(uint8_t *bytecode, uint32_t len, uint16_t ver, int debug);

#endif
