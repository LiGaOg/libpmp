#include "os.h"
extern void uart_init(void);
extern void page_init(void);
extern int pmp_init(void);
extern void pmp_mmap(unsigned int start, unsigned int end,
                     unsigned char privilege, int v_pmp_id);
extern unsigned char query_privilege(unsigned int addr);

void start_kernel(void) {

  uart_init();
  uart_puts("Hello, libpmp kernel!\n");
  page_init();

  pmp_init();
  pmp_mmap(0x87E00000, 0x87F00000, 0x6, 0);

  unsigned char privilege = query_privilege(0x87E00002);

  while (1) {
  }; // stop here!
}
