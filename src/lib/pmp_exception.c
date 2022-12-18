#include "pmp_util.h"
#include "pmp_system_library.h"
#include "pmp_exception.h"

extern void _restore_context();

void pmp_exception_handler() {
	

	/* The exception handler is running in M mode */

	/* If the exception is breakpoint, then executing refresh */
	uint32_t mcause;
	__asm__ __volatile__(
		"csrr %0, mcause"
		:"+r"(mcause)
	);

	if (mcause == 0x3) {
		for (int i = 0; i < middle->number_of_node; i ++) {
			uint32_t start = middle->cache[i]->start;
			uint32_t end = middle->cache[i]->end;
			uint8_t privilege = middle->cache[i]->privilege;
			uint8_t mask = 0x08;
			uint8_t pmpcfg_content = privilege | mask;
			write_pmpcfg(i * 2, 0);
			write_pmpcfg(i * 2 + 1, pmpcfg_content);
			write_pmpaddr(i * 2, start);
			write_pmpaddr(i * 2 + 1, end);
		}
		uint32_t mepc;
		
		/* Increment mepc by 4 to point to the next instruction of ebreak */
		__asm__ __volatile__(
			"csrr %0, mepc"
			:"+r"(mepc)
		);
		mepc += 4;
		__asm__ __volatile__(
			"csrw mepc, %0"
			::"r"(mepc)
		);
	}
	/* If not breakpoint exception, then it is PMP related */
	else {
		/* Read the invalid address from mtval */
		uint32_t addr;
		__asm__ __volatile__(
			"csrr %0, mtval"
			:"+r"(addr)
		);
		/* Find the entry which contains addr and has
		 * the highest priority in cache */
		virtual_pmp_entry *target_entry = NULL;
		for (size_t i = 0; i < middle->number_of_node; i ++) {
			uint32_t start = middle->cache[i]->start;
			uint32_t end = middle->cache[i]->end;
			if (addr2pmpaddr( addr ) >= start && addr2pmpaddr( addr ) < end) {
				target_entry = middle->cache[i];
				break;
			}
		}
		/* If this target_entry is in actual pmp, ignore it */
		if (target_entry != NULL) {
			/* Increment mepc by 4 */
			uint32_t mepc;
			
			/* Increment mepc by 4 to point to the next instruction of ebreak */
			__asm__ __volatile__(
				"csrr %0, mepc"
				:"+r"(mepc)
			);
			mepc += 4;
			__asm__ __volatile__(
				"csrw mepc, %0"
				::"r"(mepc)
			);
			return ;
		}
		/* If this target entry is not in */
		if (target_entry == NULL) {
			virtual_pmp_entry *virtual_target_entry = find_highest_priority_entry(addr);
			/* If this address is not contained by virtual pmp entry, ignore */
			/* If contained, then refresh it into physical pmp entries */
			if (virtual_target_entry != NULL) {
				add_virtual_pmp_entry_to_cache(virtual_target_entry);
				for (int i = 0; i < middle->number_of_node; i ++) {
					uint32_t start = middle->cache[i]->start;
					uint32_t end = middle->cache[i]->end;
					uint8_t privilege = middle->cache[i]->privilege;
					uint8_t mask = 0x08;
					uint8_t pmpcfg_content = privilege | mask;
					write_pmpcfg(i * 2, 0);
					write_pmpcfg(i * 2 + 1, pmpcfg_content);
					write_pmpaddr(i * 2, start);
					write_pmpaddr(i * 2 + 1, end);
				}
				/* Don't increment mepc by 4 because this instruction access needs executing again */
				return ;
			}
			else {
				/* If no target entry in linkedlist, increment mepc by 4 */
				/* Increment mepc by 4 */
				uint32_t mepc;
				
				/* Increment mepc by 4 to point to the next instruction of ebreak */
				__asm__ __volatile__(
					"csrr %0, mepc"
					:"+r"(mepc)
				);
				mepc += 4;
				__asm__ __volatile__(
					"csrw mepc, %0"
					::"r"(mepc)
				);
				return ;
			}
		}
	}
}
