#include "pmp_types.h"

/*
 * This function is used to read the value of pmpcfg register under RV32
 * params:
 * 	pmpcfg_id: 0~15, identify which pmpcfg you want to read.
 * return:
 * 	The value in the target pmpcfg */
uint8_t read_pmpcfg(int pmpcfg_id);


/*
 * This function is used to find the entry which contains addr
 * and has the highest priority
 * param:
 * 	addr: target address
 * return:
 * 	target entry, return NULL if no virtual pmp entry contain
 * 	that addr */
virtual_pmp_entry *find_highest_priority_entry(uint32_t addr) {

	/* Iterate all virtual pmp entries in linkedlist */
	virtual_pmp_entry *cur = dummy.head;
	virtual_pmp_entry *target = NULL;
	int highest_priority = -1;
	for (size_t i = 0; i < dummy.number_of_node; i ++) {
		uint32_t start = cur->start;
		uint32_t end = cur->end;
		uint8_t priority = cur->priority;
		if (addr >= start && addr <= end) {
			if (highest_priority != -1 && priority < highest_priority) {
				highest_priority = priority;
				target = cur;
			}
		}
		cur = cur->next;
	}
	return target;
}
