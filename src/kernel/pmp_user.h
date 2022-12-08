#include "types.h"
extern void pmp_free(int priority);


extern void pmp_isolation_request(uint32_t start, uint32_t end, uint8_t privilege, int priority);
