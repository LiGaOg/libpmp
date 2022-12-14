#include "pmp_system_library.h"
#include "pmp_debug.c"
#include "pmp_types.h"
#include "pmp_user.h"
#include "pmp_exception.h"


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
void pmp_isolation_request(uint32_t start, uint32_t end, uint8_t privilege, int priority) {
	
	/* Find all intersected virtual pmp entry in linkedlist and form a new linkedlist */
	virtual_pmp_entry *pmp_entry_head = NULL;
	virtual_pmp_entry *cur = dummy.head;
	for (int i = 0; i < dummy.number_of_node; i ++) {
		if (is_intersect(cur->start, cur->end, start, end)) {
			if (pmp_entry_head == NULL) {
				pmp_entry_head = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
				pmp_entry_head->start = cur->start;
				pmp_entry_head->end = cur->end;
				pmp_entry_head->priority = cur->priority;
				pmp_entry_head->privilege = cur->privilege;
				pmp_entry_head->start = pmp_entry_head->end = NULL;
				/* Deleted this node in original linkedlist */
				delete_virtual_pmp_entry(cur);
			}
			else {
				virtual_pmp_entry *newEntry = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
				newEntry->start = cur->start;
				newEntry->end = cur->end;
				newEntry->priority = cur->priority;
				newEntry->privilege = cur->privilege;
				newEntry->prev = pmp_entry_head;
				newEntry->next = pmp_entry_head->next;
				if (pmp_entry_head->next != NULL)
					pmp_entry_head->next->prev = newEntry;
				pmp_entry_head->next = newEntry;
				delete_virtual_pmp_entry(cur);
			}
		}
	}
	/* Now pmp_entry_head stores all entries intersects with [start, end] */

	/* Sort pmp_entry_head linkedlist according to the left side of interval */
	
	/* Create a virtual linkedlist head */
	virtual_pmp_entry *dummy_head = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
	dummy_head->start = dummy_head->end = -1;
	dummy_head->priority = dummy_head->priority = -1;
	dummy_head->prev = NULL;
	dummy_head->next = pmp_entry_head;
	pmp_entry_head->prev = dummy_head;

	/* Sorting algorithm is selection sort */
	virtual_pmp_entry *left_gate = dummy_head->next;
	for (; left_gate != NULL; left_gate = left_gate->next) {
		virtual_pmp_entry *cur = left_gate;
		uint32_t start_max = 0xffffffff;
		virtual_pmp_entry *target = NULL;
		/* Find the node with the minimum start */
		for (; cur != NULL; cur = cur->next) {
			if (cur->start < start_max) {
				target = cur;
				start_max = cur->start;
			}
		}
		/* Delete the node with minimum start */
		cur->prev->next = cur->next;
		cur->next->prev = cur->prev;

		/* Insert cur into the left side of gate */
		cur->prev = left_gate->prev;
		cur->next = left_gate;
		left_gate->prev->next = cur;
		left_gate->prev = cur;
	}
	/* Now dummy_head->next stores the sorted linkedlist */
	virtual_pmp_entry *organized_entry_head = NULL;	
	/* Then we need to cut the interval according to the interval side point */
	cur = dummy_head->next;
	for (; cur != NULL; cur = cur->next) {
		/* Case 1: [start, [cur->start, cur->end], end] */
		if ((start < cur->start && cur->end < end) || (start == cur->start && end == cur->end)) {
			/* Node 1 */
			virtual_pmp_entry *node1 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node1->start = cur->start;
			node1->end = cur->end;
			node1->priority = cur->priority;
			node1->privilege = cur->privilege;

			/* Node 2 */
			virtual_pmp_entry *node2 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node2->start = cur->start;
			node2->end = cur->end;
			node2->priority = priority;
			node2->privilege = cur->privilege;

			/* Insert those two nodes into new linkedlist */
			if (organized_entry_head == NULL) organized_entry_head = node1;
			else {
				node1->prev = organized_entry_head;
				node1->next = organized_entry_head->next;
				if (organized_entry_head->next != NULL)
					organized_entry_head->next->prev = node1;
				organized_entry_head->next = node1;
			}
			node2->prev = organized_entry_head;
			node2->next = organized_entry_head->next;
			if (organized_entry_head->next != NULL)
				organized_entry_head->next->prev = node2;
			organized_entry_head->next = node2;

		}
		/* Case 2: [cur->start, [start, end], cur->end] */
		else if (cur->start < start && end < cur->end) {

			/* Node1 */
			virtual_pmp_entry *node1 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node1->start = cur->start;
			node1->end = start;
			node1->priority = cur->priority;
			node1->privilege = cur->privilege;

			/* Node2 */
			virtual_pmp_entry *node2 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node2->start = start;
			node2->end = end;
			node2->priority = priority;
			node2->privilege = cur->privilege;

			/* Node3 */
			virtual_pmp_entry *node3 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node3->start = start;
			node3->end = end;
			node3->priority = cur->priority;
			node3->privilege = cur->privilege;

			/* Node4 */
			virtual_pmp_entry *node4 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node4->start = end;
			node4->end = cur->end;
			node4->priority = cur->priority;
			node4->privilege = cur->privilege;

			/* Insert those four nodes into linkedlist */
			if (organized_entry_head == NULL) organized_entry_head = node1;
			else {
				node1->prev = organized_entry_head;
				node1->next = organized_entry_head->next;
				if (organized_entry_head->next != NULL)
					organized_entry_head->next->prev = node1;
				organized_entry_head->next = node1;
			}
			node2->prev = organized_entry_head;
			node2->next = organized_entry_head->next;
			if (organized_entry_head->next != NULL)
				organized_entry_head->next->prev = node2;
			organized_entry_head->next = node2;

			node3->prev = organized_entry_head;
			node3->next = organized_entry_head->next;
			if (organized_entry_head->next != NULL)
				organized_entry_head->next->prev = node3;
			organized_entry_head->next = node3;

			node4->prev = organized_entry_head;
			node4->next = organized_entry_head->next;
			if (organized_entry_head->next != NULL)
				organized_entry_head->next->prev = node4;
			organized_entry_head->next = node4;
		}
		/* Case 3: [cur->start, start, cur->end, end] */
		else if (cur->start < start) {
			/* Node1 */
			virtual_pmp_entry *node1 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node1->start = cur->start;
			node1->end = start;
			node1->priority = cur->priority;
			node1->privilege = cur->privilege;

			/* Node2 */
			virtual_pmp_entry *node2 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node2->start = start;
			node2->end = cur->end;
			node2->priority = cur->priority;
			node2->privilege = cur->privilege;

			/* Node3 */
			virtual_pmp_entry *node3 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node3->start = start;
			node3->end = cur->end;
			node3->priority = priority;
			node3->privilege = cur->privilege;

			/* Insert those three nodes into linkedlist */
			if (organized_entry_head == NULL) organized_entry_head = node1;
			else {
				node1->prev = organized_entry_head;
				node1->next = organized_entry_head->next;
				if (organized_entry_head->next != NULL)
					organized_entry_head->next->prev = node1;
				organized_entry_head->next = node1;
			}
			node2->prev = organized_entry_head;
			node2->next = organized_entry_head->next;
			if (organized_entry_head->next != NULL)
				organized_entry_head->next->prev = node2;
			organized_entry_head->next = node2;

			node3->prev = organized_entry_head;
			node3->next = organized_entry_head->next;
			if (organized_entry_head->next != NULL)
				organized_entry_head->next->prev = node3;
			organized_entry_head->next = node3;
		}
		/* Case 4: [start, cur->start, end, cur->end] */
		else if (end < cur->end) {

			/* Node1 */
			virtual_pmp_entry *node1 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node1->start = cur->start;
			node1->end = end;
			node1->priority = cur->priority;
			node1->privilege = cur->privilege;

			/* Node2 */
			virtual_pmp_entry *node2 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node2->start = cur->start;
			node2->end = end;
			node2->priority = priority;
			node2->privilege = cur->privilege;

			/* Node3 */
			virtual_pmp_entry *node3 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node3->start = end;
			node3->end = cur->end;
			node3->priority = cur->priority;
			node3->privilege = cur->privilege;

			/* Insert those three nodes into linkedlist */
			if (organized_entry_head == NULL) organized_entry_head = node1;
			else {
				node1->prev = organized_entry_head;
				node1->next = organized_entry_head->next;
				if (organized_entry_head->next != NULL)
					organized_entry_head->next->prev = node1;
				organized_entry_head->next = node1;
			}
			node2->prev = organized_entry_head;
			node2->next = organized_entry_head->next;
			if (organized_entry_head->next != NULL)
				organized_entry_head->next->prev = node2;
			organized_entry_head->next = node2;

			node3->prev = organized_entry_head;
			node3->next = organized_entry_head->next;
			if (organized_entry_head->next != NULL)
				organized_entry_head->next->prev = node3;
			organized_entry_head->next = node3;
		}

		/* If two interval has empty area */
		if (cur->next != NULL && cur->end < cur->next->start) {
			/* Node1 */
			virtual_pmp_entry *node1 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node1->start = cur->end;
			node1->end = cur->next->start;
			node1->priority = priority;
			node1->privilege = cur->privilege;

			/* Insert this node into linkedlist */
			if (organized_entry_head == NULL) organized_entry_head = node1;
			else {
				node1->prev = organized_entry_head;
				node1->next = organized_entry_head->next;
				if (organized_entry_head->next != NULL)
					organized_entry_head->next->prev = node1;
				organized_entry_head->next = node1;
			}
		}
	}

	/* Free original linkedlist */
	virtual_pmp_entry_free(dummy_head);

	/* Find tail of linkedlist */
	cur = organized_entry_head;
	while (cur->next != NULL) cur = cur->next;
	virtual_pmp_entry *organized_entry_tail = cur;
	
	/* Insert organized virtual entry into linkedlist */
	organized_entry_head->prev = dummy.head;
	organized_entry_tail->next = dummy.head->next;
	if (dummy.head->next != NULL) dummy.head->next->prev = organized_entry_tail;
	dummy.head->next = organized_entry_head;


	/* Update cache according to specific rules */
	for (int i = 0; i < middle->number_of_node; i ++) {
		if (is_intersect(middle->cache[i]->start, middle->cache[i]->end, start, end)) {
			/* Case 1: [start, cache[i]->start, cache[i]->end, end] */
			if (start <= middle->cache[i]->start && middle->cache[i]->end <= end) {
				cur = organized_entry_head;
				int highest_priority = 0x3f3f3f3f;
				virtual_pmp_entry *target = NULL;
				/* Find small interval with highest priority which is the same as [start, end] */
				for (; cur != NULL; cur = cur->next) {
					if (cur->start == middle->cache[i]->start && cur->end == middle->cache[i]->end) {
						if (cur->priority >= highest_priority) continue;
						highest_priority = cur->priority;
						target = cur;
					}
				}
				if (target->priority < middle->cache[i]->priority) {
					delete_virtual_pmp_entry(target);
					add_virtual_pmp_entry_to_cache(target);
				}
			}
			/* Case 2: [cache[i]->start, start, end, cache[i]->end] */
			else if (middle->cache[i]->start <= start && end <= middle->cache[i]->end) {
				cur = organized_entry_head;
				int highest_priority = 0x3f3f3f3f;
				virtual_pmp_entry *target = NULL;
				/* Find small interval with highest priority which is the same as [start, end] */
				for (; cur != NULL; cur = cur->next) {
					if (cur->start == start && cur->end == end) {
						if (cur->priority >= highest_priority) continue;
						highest_priority = cur->priority;
						target = cur;
					}
				}
				if (target->priority < middle->cache[i]->priority) {
					delete_virtual_pmp_entry(target);
					add_virtual_pmp_entry_to_cache(target);
				}
			}
			/* Case 3: [start, cache[i]->start, end, cache[i]->end] */
			else if (start <= middle->cache[i]->start && end >= middle->cache[i]->end) {
				cur = organized_entry_head;
				int highest_priority = 0x3f3f3f3f;
				virtual_pmp_entry *target = NULL;
				/* Find small interval with highest priority which is the same as [start, end] */
				for (; cur != NULL; cur = cur->next) {
					if (cur->start == middle->cache[i]->start && cur->end == end) {
						if (cur->priority >= highest_priority) continue;
						highest_priority = cur->priority;
						target = cur;
					}
				}
				if (target->priority < middle->cache[i]->priority) {
					delete_virtual_pmp_entry(target);
					add_virtual_pmp_entry_to_cache(target);
				}
			}
			/* Case 4: [cache[i]->start, start, cache[i]->end, end] */
			else if (middle->cache[i]->start <= start && middle->cache[i]->end <= end) {
				cur = organized_entry_head;
				int highest_priority = 0x3f3f3f3f;
				virtual_pmp_entry *target = NULL;
				/* Find small interval with highest priority which is the same as [start, end] */
				for (; cur != NULL; cur = cur->next) {
					if (cur->start == start && cur->end == middle->cache[i]->end) {
						if (cur->priority >= highest_priority) continue;
						highest_priority = cur->priority;
						target = cur;
					}
				}
				if (target->priority < middle->cache[i]->priority) {
					delete_virtual_pmp_entry(target);
					add_virtual_pmp_entry_to_cache(target);
				}
			}
		}
	}
	
	/* Refresh */
	refresh();

}

void pmp_test_script() {
	while (1) { printf("yes"); }
}
/* void pmp_test_script() { */

/* 	/1* Tested region is [0x87E00000, 0x87F00000] *1/ */

/* 	/1* Create 17 isolated regions which don't have read permission *1/ */
	
/* 	/1* Prioirty of those regions are increaseing *1/ */
	
/* 	/1* Those regions are intersected *1/ */

/* 	/1* [0x87E00000, 0x87E00010] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00000 ), addr2pmpaddr( 0x87E00010 ), 0x0e, 0); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00004, 0x87E00020] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00004 ), addr2pmpaddr( 0x87E00020 ), 0x0e, 1); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00014, 0x87E00030] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00014 ), addr2pmpaddr( 0x87E00030 ), 0x0e, 2); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00024, 0x87E00040] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00024 ), addr2pmpaddr( 0x87E00040 ), 0x0e, 3); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00034, 0x87E00050] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00034 ), addr2pmpaddr( 0x87E00050 ), 0x0e, 4); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00044, 0x87E00060] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00044 ), addr2pmpaddr( 0x87E00060 ), 0x0e, 5); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00054, 0x87E00070] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00054 ), addr2pmpaddr( 0x87E00070 ), 0x0e, 6); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00064, 0x87E00080] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00064 ), addr2pmpaddr( 0x87E00080 ), 0x0e, 7); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00074, 0x87E00090] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00074 ), addr2pmpaddr( 0x87E00090 ), 0x0e, 8); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00084, 0x87E000a0] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00084 ), addr2pmpaddr( 0x87E000a0 ), 0x0e, 9); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E00094, 0x87E000b0] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E00094 ), addr2pmpaddr( 0x87E000b0 ), 0x0e, 10); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E000a4, 0x87E000c0] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E000a4 ), addr2pmpaddr( 0x87E000c0 ), 0x0e, 11); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E000b4, 0x87E000d0] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E000b4 ), addr2pmpaddr( 0x87E000d0 ), 0x0e, 12); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E000c4, 0x87E000e0] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E000c4 ), addr2pmpaddr( 0x87E000e0 ), 0x0e, 13); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E000d4, 0x87E000f0] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E000d4 ), addr2pmpaddr( 0x87E000f0 ), 0x0e, 14); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E000e4, 0x87E00100] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E000e4 ), addr2pmpaddr( 0x87E00100 ), 0x0e, 15); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */
/* 	/1* [0x87E000f4, 0x87E00110] *1/ */
/* 	pmp_isolation_request(addr2pmpaddr( 0x87E000f4 ), addr2pmpaddr( 0x87E00110 ), 0x0e, 16); */
/* 	visualize_linkedlist(); */
/* 	printf("======================="); */
/* 	visualize_middle_layer(); */
/* 	printf("-----------------------"); */
/* 	printf("\n"); */

/* 	/1* Free entries *1/ */
/* 	for (int i = 16; i >= 0; i --) { */
/* 		pmp_free(i); */
/* 		visualize_linkedlist(); */
/* 		printf("======================="); */
/* 		visualize_middle_layer(); */
/* 		printf("-----------------------"); */
/* 		printf("\n"); */
/* 	} */

/* 	while (1) {  } */
/* } */

/* Config csrs to support pmp virtualization */
void pmp_virtualize_init() {
	/* Initially, the machine is running in M mode */
	uint32_t sstatus_mask = 0x2;
	uint32_t sie_mask = 0x2;
	uint32_t stvec = pmp_exception_handler;
	uint32_t mepc = 0x80000008;

	/* Enable sstatus.SIE */
	uint32_t original_sstatus;
	__asm__ __volatile__(
		"csrr %0, sstatus"
		::"r"(original_sstatus)
	);
	original_sstatus |= sstatus_mask;
	__asm__ __volatile__(
		"csrw sstatus, %0"
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
		"csrw sie, %0"
		:"+r"(original_sie)
	);
	/* Config stvec to exception handler */
	__asm__ __volatile__(
		"csrw stvec, %0"
		::"r"(stvec)
	);

	/* Config mtvec to exception handler */
	__asm__ __volatile__(
		"csrw mtvec, %0"
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
