#include "pmp_util.c"
#include "pmp_system_library.h"

void pmp_exception_handler() {

	uint32_t addr;
	
	/* If the exception is breakpoint, then executing refresh */
	uint32_t mcause;
	__asm__ __volatile__(
		"csrr %0, mcause"
		:"+r"(mcause)
	);
	if (mcause == 0x3) {
		for (int i = 0; i < middle->number_of_node; i += 2) {
			uint32_t start = middle->cache[i]->start;
			uint32_t end = middle->cache[i]->end;
			uint8_t privilege = middle->cache[i]->privilege;
			uint8_t mask = 0x08;
			uint8_t pmpcfg_content = privilege | mask;
			write_pmpcfg(i, 0);
			write_pmpcfg(i + 1, pmpcfg_content);
			write_pmpaddr(i, start);
			write_pmpaddr(i + 1, end);
		}
	}
	else {
		/* Find the entry which contains addr and has
		 * the hightest priority in pmp entry*/
		virtual_pmp_entry *target_entry = NULL;
		for (size_t i = 0; i < middle->number_of_node; i ++) {
			uint32_t start = middle->cache[i]->start;
			uint32_t end = middle->cache[i]->end;
			if (addr >= start && addr <= end) {
				target_entry = middle->cache[i];
				break;
			}
		}
		/* If this target_entry is in actual pmp, ignore it */

		/* If this target entry is not in */
		if (target_entry == NULL) {
			virtual_pmp_entry *virtual_target_entry = find_highest_priority_entry(addr);
			/* If this address is not contained by virtual pmp entry, ignore */
			/* If contained, then refresh it into physical pmp entries */
			if (virtual_target_entry != NULL) {
				delete_virtual_pmp_entry(virtual_target_entry);
				add_virtual_pmp_entry_to_cache(virtual_target_entry);
				refresh();
			}
		}
	}
	/* Switch to S mode */
	__asm__ __volatile__("mret");
}
