#ifndef __PATCHER_H
#define __PATCHER_H

/* Patches firmware with the current configuration.
 *
 * \return zero on success
 */
int patch_firm_all(uint64_t tid, firm_h* firm);

/* Generates patch cache for boot/loader for the current configuration.
 *
 * \return Zero on success.
 */
int generate_patch_cache();

#endif
