#ifndef __PATCHER_H
#define __PATCHER_H

/* Patches firmware with the current configuration.
 *
 * \return zero on success
 */
int patch_firm_all(uint64_t tid, firm_h* firm, const char* module_path);

/* Generates patch cache for boot/loader for the current configuration.
 *
 * \return Zero on success.
 */
int generate_patch_cache();

int patch_svc_calls(firm_h* firm_loc);
int patch_reboot(firm_h* firm_loc);
int patch_modules(firm_h* firm_loc, const char* module_path);

#endif
