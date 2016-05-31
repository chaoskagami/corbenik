#include "patch_file.h"

// Do you like examples?

/* Bytecode assembly:

   example:
     mov r1, 2
     mov r2, string
     call fprintf
     mov r1, 0
     return

   string:
     .str "Testing, testing, 1, 2, 3...\n"
 */

PATCH(example)
{
    fprintf(stderr, "Testing, testing, 1, 2, 3, 4..\n");

    return 0;
}
