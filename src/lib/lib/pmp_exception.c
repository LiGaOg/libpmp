#include "pmp_util.c"
#include "pmp_types.h"
#include "pmp_system_library.h"

void pmp_exception_handler(uint32_t addr) {
	
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
