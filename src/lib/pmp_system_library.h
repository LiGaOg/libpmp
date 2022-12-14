#ifndef __PMP_SYSTEM_LIBRARY_H__
#define __PMP_SYSTEM_LIBRARY_H__
#include "pmp_types.h"

extern void *malloc(uint32_t size);

extern void free(void *ptr);

extern int printf(const char* s, ...);
#endif
