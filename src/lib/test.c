#include "pmp_user.h"

int main()
{
	pmp_mmap((unsigned int)0x80000000, 
		(unsigned int)0x80000010, 
		0x3f, 
		1145);
	return 0;
}
