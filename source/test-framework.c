#include <corbconf.h>
#if defined(UNITTESTS) && UNITTESTS == 1

#include <common.h>

uint32_t total;
uint32_t pass;
uint32_t fail;
uint32_t skip;

void test(int value, const char* name) {
    fprintf(stderr, "%s: ", name);

    switch(value) {
        case 0:
            fprintf(stderr, "pass\n");
            ++pass;
            break;
        case 1:
            fprintf(stderr, "fail\n");
            ++fail;
            break;
        default:
            fprintf(stderr, "skip\n");
            ++skip;
            break;
    }
    ++total;
}

int assert_null(void* ptr) {
	if (ptr == NULL)
		return 0;
	return 1;
}

int assert_nonnull(void* ptr) {
	if (ptr != NULL)
		return 0;
	return 1;
}

int assert_int(int input, int value) {
    if (input != value)
        return 1;
    return 0;
}

int assert_u32(uint32_t input, uint32_t value) {
    if (input != value)
        return 1;
    return 0;
}

int assert_array_eq(uint8_t* input, uint8_t* test, uint32_t len) {
    for (uint32_t i=0; i < len; i++) {
        if (input[i] != test[i])
            return 1;
    }
    return 0;
}

// We do NOT want the test harness optimized whatsoever.

void __attribute__((optimize("O0"))) run_test_harness() {
    total = pass = fail = skip = 0;

	#include "test/basic-sanity.test"
	#include "test/file.test"
//	#include "test/firmware.test"
//	#include "test/vm.test"
//	#include "test/vm-patch.test"

	fprintf(stderr, "= Tests =\n"
                    "Total: %lu\n"
                    "Pass:  %lu\n"
                    "Fail:  %lu\n"
                    "Skip:  %lu\n",
                    total, pass, fail, skip);
}

#endif
