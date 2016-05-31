#ifndef __LOGGER_H
#define __LOGGER_H

void openLogger();
void logstr(const char* str);
void logu64(u64 progId);
void closeLogger();

#endif
