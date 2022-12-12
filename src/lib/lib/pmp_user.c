#include "pmp_system_library.h"
#include "pmp_exception.c"
#include "pmp_types.h"

extern void pmp_test_script();
/* Config csrs to support pmp virtualization */
void pmp_virtualize_init() {
	/* Initially, the machine is running in M mode */
	uint32_t sstatus_mask = 0x2;
	uint32_t sie_mask = 0x2;
	uint32_t stvec = pmp_exception_handler;
	uint32_t mepc = pmp_test_script;

	/* Enable sstatus.SIE */
	uint32_t original_sstatus;
	__asm__ __volatile__(
		"csrr %0, sstatus"
		::"r"(original_sstatus)
	);
	original_sstatus |= sstatus_mask;
	__asm__ __volatile__(
		"csrw sstatus %0"
		:"+r"(original_sstatus)
	);
	/* Enable sie.SSIP */
	uint32_t original_sie;
	__asm__ __volatile__(
		"csrr %0, sie"
		::"r"(original_sie)
	);
	original_sie |= sie_mask;
	__asm__ __volatile__(
		"csrw sie %0"
		:"+r"(original_sie)
	);
	/* Config stvec to exception handler */
	__asm__ __volatile__(
		"csrw stvec, %0"
		::"r"(stvec)
	);
	/* Other virtualization work in libpmp */
	init_middle_layer();

	/* Config mepc to kernel */
	__asm__ __volatile__(
		"csrw mepc, %0"
		::"r"(mepc)
	);
	/* Switch to S mode and jump to kernel */
	__asm__ __volatile__(
		"mret"
	);
}
/* Free one virtual pmp entry according to priorty */
void pmp_free(int priority) {
	
	/* Iterate and delete virtual entries in linkedlist */
	virtual_pmp_entry *cur = dummy.head;
	for (size_t i = 0; i < dummy.number_of_node; i ++) {
		if (cur->priority == priority) {
			delete_virtual_pmp_entry(cur);
		}
		cur = cur->next;
	}

	/* Iterate and delete virtual entries in middle layer */
	for (size_t i = 0; i < middle->number_of_node; i ++) {
		if (middle->cache[i]->priority == priority) {
			delete_middle_layer_entry(middle->cache[i]);
		}
	}
	/* refresh to physical PMP entries */
	refresh();
}

/* Create a pmp isolation request */
void pmp_isolation_request(uint32_t start, uint32_t end, uint8_t privilege, int priority);
