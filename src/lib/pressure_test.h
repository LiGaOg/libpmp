/* This file is the pressure test header
 * Include the header, and run muti_test()*/
#include "pmp_system_library.h"
#include "pmp_user.h"

extern void muti_test();

extern void testNoOverlap(unsigned int request_num);

extern void testSequentialOverlap(unsigned int request_num, unsigned int space_len, int overlap_ratio);

extern void testCentricOverlap(unsigned int request_num, unsigned int init_space_len, unsigned int extend_radius);

extern void testHighOverlap(unsigned int request_num, unsigned int block_interval);

extern unsigned int read_cycles(void);
