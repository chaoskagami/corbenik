#ifndef __ABORT_H
#define __ABORT_H

#include "draw.h"

#define abort(x...) { \
  fprintf(stderr, x); \
  fumount(); \
  while(1); \
}

#endif
