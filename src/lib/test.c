#include "pmp_heap.h"
#include "pmp_user.h"


static unsigned int query_start[] = {
	0x80000000,
	0x80000010,
	0x80000020,
	0x80000030,
	0x80000040,
	0x80000050,
	0x80000060,
	0x80000070,
	0x80000080,
	0x80000090,
	0x800000a0,
	0x800000b0,
	0x800000c0,
	0x800000d0,
	0x800000e0,
	0x800000f0
};


static unsigned int query_end[] = {
	0x80000010,
	0x80000020,
	0x80000030,
	0x80000040,
	0x80000050,
	0x80000060,
	0x80000070,
	0x80000080,
	0x80000090,
	0x800000a0,
	0x800000b0,
	0x800000c0,
	0x800000d0,
	0x800000e0,
	0x800000f0,
	0x80000100
};

int main()
{
	pmp_init();

	for (int i = 0; i < 16; i++) {
		
		pmp_mmap((unsigned int)query_start[i], 
			(unsigned int)query_end[i], 
			(unsigned char)i, 
			i);
	}
	log();
	for (int i = 0; i < 9; i++) {
		unsigned char result = query_privilege((unsigned int)(query_start[i] + 5));
		printf("query result=%x\n", result);
		log();
		printf("----------%d---------------\n", i);
	}

	return 0;
}
