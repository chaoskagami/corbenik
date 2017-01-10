// This is a thin shim used to set the right parameters and include the interpreter from toplevel code.
#include <3ds.h>
#include <stdlib.h>
#include "patcher.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "memory.h"
#include "logger.h"

#ifndef PATH_MAX
#define PATH_MAX 255
#define _MAX_LFN 255
#endif
#include <option.h>
#include <structures.h>

// Yes, we're including a C file. Problem?
#include "../../../source/interpreter.c"

