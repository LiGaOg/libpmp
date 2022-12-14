#include "os.h"
#include "pmp_user.h"

extern void uart_init(void);
extern void page_init(void);

void start_kernel(void) {

	uart_init();
	uart_puts("Hello, libpmp kernel!\n");
	page_init();

	pmp_virtualize_init();
	while (1) {
	}; // stop here!
}
