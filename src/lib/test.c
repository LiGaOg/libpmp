#include "pmp_heap.h"
#include "pmp_user.h"

int main()
{
	pmp_init();
	pmp_mmap((unsigned int)0x80000000, 
		(unsigned int)0x80000010, 
		(unsigned char)0x3f, 
		1145);
	log();
	query_privilege((unsigned int)0x80000005);
	log();
	return 0;
}
