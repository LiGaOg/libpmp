#ifndef __PMP_USER_H__
#define __PMP_USER_H__
#include "pmp_types.h"

/* Free one virtual pmp entry according to priorty */
extern void pmp_free(int priority); 

/* Create a pmp isolation request */
extern void pmp_isolation_request(uint32_t start, uint32_t end, uint8_t privilege, int priority);

extern void pmp_test_script(); 

/* Config csrs to support pmp virtualization */
extern void pmp_virtualize_init();

#endif
