#include "pmp_system_library.h"
#include "pmp_util.c"

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
