#include "pmp_debug.h"
#include "pmp_system_library.h"
#include "pmp_types.h"
#include "pmp_util.h"

void visualize_linkedlist() {
	virtual_pmp_entry *cur = dummy.head;
	if (dummy.number_of_node == 0 && dummy.head == NULL) {
		printf("linkedlist empty\n");
	}
	for (int i = 0; i < dummy.number_of_node; i ++) {
		printf("start:%x\n", cur->start << 2);
		printf("end:%x\n", cur->end << 2);
		printf("privilege:%x\n", cur->privilege);
		printf("priority:%x\n", cur->priority);
		if (i != dummy.number_of_node - 1) {
			printf("|\n");
			printf("|\n");
		}
		cur = cur->next;
	}
}
void visualize_middle_layer() {
	if (middle->number_of_node == 0) {
		printf("cache empty\n");
	}
	for (int i = 0; i < middle->number_of_node; i ++) {
		if (middle->cache[i] == NULL) printf("cache[%d]:NULL\n", i);
		else {
			printf("cache[%d]:\n", i);
			printf("\tstart:%x\n", middle->cache[i]->start);
			printf("\tend:%x\n", middle->cache[i]->end);
			printf("\tprivilege:%x\n", middle->cache[i]->privilege);
			printf("\tpriority:%x\n", middle->cache[i]->priority);
		}
	}
}

void visualize_physical_pmp_entries() {
	for (int i = 0; i < 16; i ++) {
		printf("pmp%dcfg:%x pmpaddr%d:%x\n", i, read_pmpcfg(i), i, read_pmpaddr(i));
	}
}
