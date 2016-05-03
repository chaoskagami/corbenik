#include <stdint.h>
#include "std/unused.h"
#include "std/memory.h"
#include "config.h"

#define SIGNATURE_CHECK     {0x00, 0x20}
#define SIGNATURE_CHECK_FIX {0x70, 0x60, 0x70, 0x47}

int patch_firm_all() {
	// Use builtin signature patcher?
	if (config.options[OPTION_SIGPATCH]) {
		// Yes.
		// memfind(
	}

	// Replace loader?
	if (config.options[OPTION_LOADER]) {
		// Yes.

		// This requires OPTION_SIGPATCH.
	}

	// Use ARM9 hook thread?
	if (config.options[OPTION_ARM9THREAD]) {
		// Yes.
		// FIXME - NYI
	}

	return 0;
}
