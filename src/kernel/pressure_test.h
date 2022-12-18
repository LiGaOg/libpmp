/* This file is the pressure test header
 * Include the header, and run muti_test()*/

void muti_test();

void testNoOverlap(unsigned int request_num);

void testSequentialOverlap(unsigned int request_num, unsigned int space_len, float overlap_ratio);

void testCentricOverlap(unsigned int request_num, unsigned int init_space_len, unsigned int extend_radius);

void testHighOverlap();

unsigned long read_cycles(void);
