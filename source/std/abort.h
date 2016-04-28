#ifndef __ABORT_H
#define __ABORT_H

#define abort(x...) { \
  fprintf(2, x); \
  fumount(); \
  while(1); \
}

#endif
