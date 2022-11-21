#include "os.h"
extern void uart_init(void);
extern void page_init(void);
extern int pmp_init(void);
extern void pmp_mmap(unsigned int start, unsigned int end, unsigned char privilege, int v_pmp_id);

void start_kernel(void)
{
	uart_init();
	uart_puts("Hello, RVOS!\n");
	page_init();

	pmp_init();
	pmp_mmap(0x80000000, 0x80000000);


	while (1) {}; // stop here!
}

