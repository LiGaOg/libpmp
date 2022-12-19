#include "pmp_system_library.h"
#include "pmp_debug.h"
#include "pmp_types.h"
#include "pmp_user.h"
#include "pmp_exception.h"
#include "pmp_util.h"
#include "pressure_test.h"


extern void _save_context();

/* Free one virtual pmp entry according to priorty */
void pmp_free(int priority) {
	
	/* Iterate and delete virtual entries in linkedlist */
	virtual_pmp_entry *cur = dummy.head;
	for (size_t i = 0; i < dummy.number_of_node; i ++) {
		virtual_pmp_entry *tmp = cur->next;
		if (cur->priority == priority) {
			delete_virtual_pmp_entry(cur);
		}
		cur = tmp;
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
	int num = dummy.number_of_node;
	for (int i = 0; i < num; i ++) {
		if (is_intersect(cur->start, cur->end, start, end)) {
			if (pmp_entry_head == NULL) {
				pmp_entry_head = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
				pmp_entry_head->start = cur->start;
				pmp_entry_head->end = cur->end;
				pmp_entry_head->priority = cur->priority;
				pmp_entry_head->privilege = cur->privilege;
				pmp_entry_head->prev = pmp_entry_head->next = NULL;
				/* Deleted this node in original linkedlist */
				virtual_pmp_entry *tmp = cur;
				cur = cur->next;
				delete_virtual_pmp_entry(tmp);
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
				virtual_pmp_entry *tmp = cur;
				cur = cur->next;
			}
		}
	}
	/* Now pmp_entry_head stores all entries intersects with [start, end] */

	/* If pmp_entry_head is NULL, then no intersection found */
	if (pmp_entry_head == NULL) {
		/* Firstly, create a node */
		virtual_pmp_entry *newEntry = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
		newEntry->start = start;
		newEntry->end = end;
		newEntry->priority = priority;
		newEntry->privilege = privilege;
		
		/* Insert this new node into linkedlist */
		add_virtual_pmp_entry(newEntry);

		/* Insert into cache */
		add_virtual_pmp_entry_to_cache(newEntry);

		refresh();
		return ;

	}

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
	for (; left_gate->next != NULL; ) {
		virtual_pmp_entry *cur = left_gate->next;
		uint32_t start_max = 0xffffffff;
		virtual_pmp_entry *target = NULL;

		if (cur == NULL) break;
		/* Find the node with the minimum start */
		for (; cur != NULL; cur = cur->next) {
			if (cur->start < start_max) {
				target = cur;
				start_max = cur->start;
			}
		}
		/* Delete the node with minimum start */
		target->prev->next = target->next;
		if (target->next != NULL)
			target->next->prev = target->prev;

		/* Insert target into the left side of gate */
		target->prev = left_gate->prev;
		target->next = left_gate;
		left_gate->prev->next = target;
		left_gate->prev = target;
	}
	/* Now dummy_head->next stores the sorted linkedlist */
	virtual_pmp_entry *organized_entry_head = NULL;	
	/* Then we need to cut the interval according to the interval side point */
	cur = dummy_head->next;
	int flag = 0;
	for (; cur != NULL; cur = cur->next) {
		/* Case 1: [start, [cur->start, cur->end], end] */
		if (start <= cur->start && cur->end <= end) {
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
			node2->privilege = privilege;

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
			
			/* If this interval is the first interval and start < cur->start, there's an empty area */

			if (flag == 0 && start < cur->start) {
				
				/* Node3 */
				virtual_pmp_entry *node3 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
				node3->start = start;
				node3->end = cur->start;
				node3->priority = priority;
				node3->privilege = privilege;
				node3->prev = organized_entry_head;
				node3->next = organized_entry_head->next;
				if (organized_entry_head->next != NULL)
					organized_entry_head->next->prev = node3;
				organized_entry_head->next = node3;
			}

		}
		/* Case 2: [cur->start, [start, end], cur->end] */
		else if (cur->start <= start && end <= cur->end) {

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
			node2->privilege = privilege;

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
		else if (cur->start <= start) {
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
			node3->privilege = privilege;

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
		else if (end <= cur->end) {

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
			node2->privilege = privilege;

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
			node1->privilege = privilege;

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

		if (cur->next == NULL && cur->end < end) {
			
			/* Node1 */
			virtual_pmp_entry *node1 = (virtual_pmp_entry *)malloc(sizeof(virtual_pmp_entry));
			node1->start = cur->end;
			node1->end = end;
			node1->priority = priority;
			node1->privilege = privilege;

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
		flag ++;
	}

	/* Free original linkedlist */
	virtual_pmp_entry_free(dummy_head);

	/* Find tail of linkedlist and count the number of node */
	cur = organized_entry_head;
	int cnt = 1;
	while (cur->next != NULL) { 
		cur = cur->next;
		cnt ++;
	}
	virtual_pmp_entry *organized_entry_tail = cur;
	
	/* Insert organized virtual entry into linkedlist */
	organized_entry_head->prev = NULL;
	organized_entry_tail->next = dummy.head;

	if (dummy.head != NULL) dummy.head->prev = organized_entry_tail;
	dummy.head = organized_entry_head;

	dummy.number_of_node += cnt;


	/* Update cache according to specific rules */
	for (int i = 0; i < middle->number_of_node; i ++) {
		if (is_intersect(middle->cache[i]->start, middle->cache[i]->end, start, end)) {
			/* Case 1: [start, cache[i]->start, cache[i]->end, end] */
			if (start <= middle->cache[i]->start && middle->cache[i]->end <= end) {
				cur = organized_entry_head;
				int highest_priority = 0x3f3f3f3f;
				virtual_pmp_entry *target = NULL;
				int index = -1;
				/* Find small interval with highest priority which is the same as [start, end] */
				for (; cur != NULL; cur = cur->next) {
					if (cur->start == middle->cache[i]->start && cur->end == middle->cache[i]->end) {
						if (cur->priority >= highest_priority) continue;
						highest_priority = cur->priority;
						target = cur;
						index = i;
					}
				}
				if (target->priority < middle->cache[i]->priority) {
					middle->cache[index] = target;
				}
			}
			/* Case 2: [cache[i]->start, start, end, cache[i]->end] */
			else if (middle->cache[i]->start <= start && end <= middle->cache[i]->end) {
				cur = organized_entry_head;
				int highest_priority = 0x3f3f3f3f;
				virtual_pmp_entry *target = NULL;
				int index = -1;
				/* Find small interval with highest priority which is the same as [start, end] */
				for (; cur != NULL; cur = cur->next) {
					if (cur->start == start && cur->end == end) {
						if (cur->priority >= highest_priority) continue;
						highest_priority = cur->priority;
						target = cur;
						index = i;
					}
				}
				if (target->priority < middle->cache[i]->priority) {
					middle->cache[i] = target;
				}
			}
			/* Case 3: [start, cache[i]->start, end, cache[i]->end] */
			else if (start <= middle->cache[i]->start && end >= middle->cache[i]->end) {
				cur = organized_entry_head;
				int highest_priority = 0x3f3f3f3f;
				virtual_pmp_entry *target = NULL;
				int index = -1;
				/* Find small interval with highest priority which is the same as [start, end] */
				for (; cur != NULL; cur = cur->next) {
					if (cur->start == middle->cache[i]->start && cur->end == end) {
						if (cur->priority >= highest_priority) continue;
						highest_priority = cur->priority;
						target = cur;
						index = i;
					}
				}
				if (target->priority < middle->cache[i]->priority) {
					middle->cache[i] = target;
				}
			}
			/* Case 4: [cache[i]->start, start, cache[i]->end, end] */
			else if (middle->cache[i]->start <= start && middle->cache[i]->end <= end) {
				cur = organized_entry_head;
				int highest_priority = 0x3f3f3f3f;
				virtual_pmp_entry *target = NULL;
				int index = -1;
				/* Find small interval with highest priority which is the same as [start, end] */
				for (; cur != NULL; cur = cur->next) {
					if (cur->start == start && cur->end == middle->cache[i]->end) {
						if (cur->priority >= highest_priority) continue;
						highest_priority = cur->priority;
						target = cur;
						index = i;
					}
				}
				if (target->priority < middle->cache[i]->priority) {
					middle->cache[i] = target;
				}
			}
		}
	}
	
	/* Sort cache */
	adjust_middle_layer();
	/* Refresh */
	refresh();

}

void jump_target() {
	while (1) {}
}

void test_case1() {


	/* [0x87E00004, 0x87E00018), _wx, 3 */
	uint32_t addr1 = addr2pmpaddr(0x87E00004);
	uint32_t addr2 = addr2pmpaddr(0x87E00018);
	pmp_isolation_request(addr1, addr2, 0x6, 3);


	/* [0x87E00008, 0x87E00014), rw_, 2*/
	addr1 = addr2pmpaddr(0x87E00008);
	addr2 = addr2pmpaddr(0x87E00014);
	pmp_isolation_request(addr1, addr2, 0x3, 2);


	/* [0x87E0000C, 0x87E00010), _wx, 1*/
	addr1 = addr2pmpaddr(0x87E0000C);
	addr2 = addr2pmpaddr(0x87E00010);
	pmp_isolation_request(addr1, addr2, 0x1, 1);

	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase1.1:%d, content:%x\n", content != 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase1.2:%d, content:%x\n", content == 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase1.3:%d, content:%x\n", content != 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase1.4:%d, content:%x\n", content == 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase1.5:%d, content:%x\n", content != 0xee, content);

	/* /1* Attempt to read 0x87E00006 *1/ */
	/* uint8_t *ptr1 = (uint8_t *)0x87E00006; */
	/* uint8_t content1 = *(ptr1); */
	/* printf("testcase1.1 read 0x87E00006: %x, should be: %x, ok? %d\n", content1, 0x11, content1 != 0x11); */
	
	/* /1* Attempt to read 0x87E0000A *1/ */
	/* ptr1 = (uint8_t *)0x87E0000A; */
	/* content1 = *(ptr1); */
	/* printf("testcase1.2 read 0x87E0000A: %x, should be: %x, ok? %d\n", content1, 0x45, content1 == 0x45); */

	/* /1* Attempt to read 0x87E0000E *1/ */
	/* ptr1 = (uint8_t *)0x87E0000E; */
	/* content1 = *(ptr1); */
	/* printf("testcase1.3 read 0x87E0000E: %x, should be: %x, ok? %d\n", content1, 0x14, content1 == 0x14); */

	/* /1* Attempt to read 0x87E00012 *1/ */
	/* ptr1 = (uint8_t *)0x87E00012; */
	/* content1 = *(ptr1); */
	/* printf("testcase1.4 read 0x87E00012: %x, should be %x, ok? %d\n", content1, 0x19, content1 == 0x19); */

	/* /1* Attempt to read 0x87E00016 *1/ */
	/* ptr1 = (uint8_t *)0x87E00016; */
	/* content1 = *(ptr1); */
	/* printf("testcase1.5 read 0x87E00016: %x, should be %x, ok? %d\n", content1, 0x81, content1 != 0x81); */
	
	printf("Test 1 PASSED\n");
	jump_target();
}

void test_case2() {

	uint32_t addr1 = addr2pmpaddr(0x87E00004);
	uint32_t addr2 = addr2pmpaddr(0x87E00018);
	pmp_isolation_request(addr1, addr2, 0x6, 1);


	addr1 = addr2pmpaddr(0x87E00008);
	addr2 = addr2pmpaddr(0x87E00014);
	pmp_isolation_request(addr1, addr2, 0x3, 2);


	addr1 = addr2pmpaddr(0x87E0000C);
	addr2 = addr2pmpaddr(0x87E00010);
	pmp_isolation_request(addr1, addr2, 0x1, 3);

	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase2.1:%d, content:%x\n", content != 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase2.2:%d, content:%x\n", content != 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase2.3:%d, content:%x\n", content != 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase2.4:%d, content:%x\n", content != 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase2.5:%d, content:%x\n", content != 0xee, content);

	printf("Test 2 PASSED\n");
	jump_target();

}

void test_case3() {

	uint32_t addr1 = addr2pmpaddr(0x87E00008);
	uint32_t addr2 = addr2pmpaddr(0x87E00014);
	pmp_isolation_request(addr1, addr2, 0x5, 1);


	addr1 = addr2pmpaddr(0x87E00008);
	addr2 = addr2pmpaddr(0x87E00010);
	pmp_isolation_request(addr1, addr2, 0x6, 3);


	addr1 = addr2pmpaddr(0x87E00014);
	addr2 = addr2pmpaddr(0x87E00018);
	pmp_isolation_request(addr1, addr2, 0x7, 4);

	addr1 = addr2pmpaddr(0x87E00004);
	addr2 = addr2pmpaddr(0x87E0000C);
	pmp_isolation_request(addr1, addr2, 0x3, 2);

	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase3.1:%d, content:%x\n", content == 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase3.2:%d, content:%x\n", content != 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase3.3:%d, content:%x\n", content != 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase3.4:%d, content:%x\n", content != 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase3.5:%d, content:%x\n", content == 0xee, content);

	printf("Test 3 PASSED\n");
	jump_target();

}


void test_case4() {

	uint32_t addr1 = addr2pmpaddr(0x87E00008);
	uint32_t addr2 = addr2pmpaddr(0x87E00014);
	pmp_isolation_request(addr1, addr2, 0x5, 4);


	addr1 = addr2pmpaddr(0x87E00008);
	addr2 = addr2pmpaddr(0x87E00010);
	pmp_isolation_request(addr1, addr2, 0x6, 2);


	addr1 = addr2pmpaddr(0x87E00014);
	addr2 = addr2pmpaddr(0x87E00018);
	pmp_isolation_request(addr1, addr2, 0x7, 1);

	addr1 = addr2pmpaddr(0x87E00004);
	addr2 = addr2pmpaddr(0x87E0000C);
	pmp_isolation_request(addr1, addr2, 0x3, 3);

	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase4.1:%d, content:%x\n", content == 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase4.2:%d, content:%x\n", content != 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase4.3:%d, content:%x\n", content != 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase4.4:%d, content:%x\n", content != 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase4.5:%d, content:%x\n", content == 0xee, content);

	printf("Test 4 PASSED\n");
	jump_target();

}
void test_case5() {

	uint32_t addr1 = addr2pmpaddr(0x87E00004);
	uint32_t addr2 = addr2pmpaddr(0x87E00008);
	pmp_isolation_request(addr1, addr2, 0x5, 1);


	addr1 = addr2pmpaddr(0x87E00008);
	addr2 = addr2pmpaddr(0x87E0000C);
	pmp_isolation_request(addr1, addr2, 0x1, 2);


	addr1 = addr2pmpaddr(0x87E0000C);
	addr2 = addr2pmpaddr(0x87E00010);
	pmp_isolation_request(addr1, addr2, 0x6, 3);

	addr1 = addr2pmpaddr(0x87E00010);
	addr2 = addr2pmpaddr(0x87E00014);
	pmp_isolation_request(addr1, addr2, 0x2, 4);

	addr1 = addr2pmpaddr(0x87E00004);
	addr2 = addr2pmpaddr(0x87E00018);
	pmp_isolation_request(addr1, addr2, 0x7, 5);


	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase5.1:%d, content:%x\n", content != 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase5.2:%d, content:%x\n", content != 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase5.3:%d, content:%x\n", content != 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase5.4:%d, content:%x\n", content != 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase5.5:%d, content:%x\n", content == 0xee, content);

	printf("Test 5 PASSED\n");
	jump_target();

}


void test_case6() {

	uint32_t addr1 = addr2pmpaddr(0x87E00004);
	uint32_t addr2 = addr2pmpaddr(0x87E00008);
	pmp_isolation_request(addr1, addr2, 0x5, 5);


	addr1 = addr2pmpaddr(0x87E00008);
	addr2 = addr2pmpaddr(0x87E0000C);
	pmp_isolation_request(addr1, addr2, 0x1, 4);


	addr1 = addr2pmpaddr(0x87E0000C);
	addr2 = addr2pmpaddr(0x87E00010);
	pmp_isolation_request(addr1, addr2, 0x6, 3);

	addr1 = addr2pmpaddr(0x87E00010);
	addr2 = addr2pmpaddr(0x87E00014);
	pmp_isolation_request(addr1, addr2, 0x2, 2);

	addr1 = addr2pmpaddr(0x87E00004);
	addr2 = addr2pmpaddr(0x87E00018);
	pmp_isolation_request(addr1, addr2, 0x7, 1);


	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase6.1:%d, content:%x\n", content == 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase6.2:%d, content:%x\n", content == 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase6.3:%d, content:%x\n", content == 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase6.4:%d, content:%x\n", content == 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase6.5:%d, content:%x\n", content == 0xee, content);

	printf("Test 6 PASSED\n");
	jump_target();

}
void test_case7() {

	uint32_t addr1 = addr2pmpaddr(0x87E00004);
	uint32_t addr2 = addr2pmpaddr(0x87E00010);
	pmp_isolation_request(addr1, addr2, 0x6, 1);


	addr1 = addr2pmpaddr(0x87E00010);
	addr2 = addr2pmpaddr(0x87E0001C);
	pmp_isolation_request(addr1, addr2, 0x1, 3);


	addr1 = addr2pmpaddr(0x87E0000C);
	addr2 = addr2pmpaddr(0x87E00018);
	pmp_isolation_request(addr1, addr2, 0x7, 2);

	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase7.1:%d, content:%x\n", content != 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase7.2:%d, content:%x\n", content != 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase7.3:%d, content:%x\n", content != 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase7.4:%d, content:%x\n", content == 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase7.5:%d, content:%x\n", content == 0xee, content);

	ptr = (uint8_t *)(0x87E0001A);
	/* write */
	*(ptr) = 0xff;
	/* read */
	content = *(ptr);
	printf("testcase7.6:%d, content:%x\n", content != 0xff, content);

	printf("Test 7 PASSED\n");
	jump_target();

}



void test_case8() {

	uint32_t addr1 = addr2pmpaddr(0x87E00004);
	uint32_t addr2 = addr2pmpaddr(0x87E00010);
	pmp_isolation_request(addr1, addr2, 0x6, 3);


	addr1 = addr2pmpaddr(0x87E00010);
	addr2 = addr2pmpaddr(0x87E0001C);
	pmp_isolation_request(addr1, addr2, 0x1, 1);


	addr1 = addr2pmpaddr(0x87E0000C);
	addr2 = addr2pmpaddr(0x87E00018);
	pmp_isolation_request(addr1, addr2, 0x7, 2);




	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase8.1:%d, content:%x\n", content != 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase8.2:%d, content:%x\n", content != 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase8.3:%d, content:%x\n", content == 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase8.4:%d, content:%x\n", content != 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase8.5:%d, content:%x\n", content != 0xee, content);

	ptr = (uint8_t *)(0x87E0001A);
	/* write */
	*(ptr) = 0xff;
	/* read */
	content = *(ptr);
	printf("testcase8.6:%d, content:%x\n", content != 0xff, content);

	printf("Test 8 PASSED\n");
	jump_target();

}
void test_case9() {

	uint32_t addr1 = addr2pmpaddr(0x87E00008);
	uint32_t addr2 = addr2pmpaddr(0x87E0000C);
	pmp_isolation_request(addr1, addr2, 0x0, 1);


	addr1 = addr2pmpaddr(0x87E00010);
	addr2 = addr2pmpaddr(0x87E00014);
	pmp_isolation_request(addr1, addr2, 0x5, 2);


	addr1 = addr2pmpaddr(0x87E00018);
	addr2 = addr2pmpaddr(0x87E0001C);
	pmp_isolation_request(addr1, addr2, 0x6, 4);


	addr1 = addr2pmpaddr(0x87E00004);
	addr2 = addr2pmpaddr(0x87E00020);
	pmp_isolation_request(addr1, addr2, 0x7, 3);

	visualize_linkedlist();


	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase9.1:%d, content:%x\n", content == 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase9.2:%d, content:%x\n", content != 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase9.3:%d, content:%x\n", content == 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase9.4:%d, content:%x\n", content != 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase9.5:%d, content:%x\n", content == 0xee, content);

	ptr = (uint8_t *)(0x87E0001A);
	/* write */
	*(ptr) = 0xff;
	/* read */
	content = *(ptr);
	printf("testcase9.6:%d, content:%x\n", content == 0xff, content);

	ptr = (uint8_t *)(0x87E0001E);
	/* write */
	*(ptr) = 0x99;
	/* read */
	content = *(ptr);
	printf("testcase9.7:%d, content:%x\n", content == 0x99, content);

	printf("Test 9 PASSED\n");
	jump_target();

}
void test_case10() {

	uint32_t addr1 = addr2pmpaddr(0x87E00008);
	uint32_t addr2 = addr2pmpaddr(0x87E0000C);
	pmp_isolation_request(addr1, addr2, 0x0, 4);


	addr1 = addr2pmpaddr(0x87E00010);
	addr2 = addr2pmpaddr(0x87E00014);
	pmp_isolation_request(addr1, addr2, 0x5, 3);


	addr1 = addr2pmpaddr(0x87E00018);
	addr2 = addr2pmpaddr(0x87E0001C);
	pmp_isolation_request(addr1, addr2, 0x6, 1);


	addr1 = addr2pmpaddr(0x87E00004);
	addr2 = addr2pmpaddr(0x87E00020);
	pmp_isolation_request(addr1, addr2, 0x7, 2);

	visualize_linkedlist();


	uint8_t *ptr;
	uint8_t content;

	ptr = (uint8_t *)(0x87E00006);
	/* write */
	*(ptr) = 0xaa;
	/* read */
	content = *(ptr);
	printf("testcase10.1:%d, content:%x\n", content == 0xaa, content);

	ptr = (uint8_t *)(0x87E0000A);
	/* write */
	*(ptr) = 0xbb;
	/* read */
	content = *(ptr);
	printf("testcase10.2:%d, content:%x\n", content == 0xbb, content);

	ptr = (uint8_t *)(0x87E0000E);
	/* write */
	*(ptr) = 0xcc;
	/* read */
	content = *(ptr);
	printf("testcase10.3:%d, content:%x\n", content == 0xcc, content);


	ptr = (uint8_t *)(0x87E00012);
	/* write */
	*(ptr) = 0xdd;
	/* read */
	content = *(ptr);
	printf("testcase10.4:%d, content:%x\n", content == 0xdd, content);

	ptr = (uint8_t *)(0x87E00016);
	/* write */
	*(ptr) = 0xee;
	/* read */
	content = *(ptr);
	printf("testcase10.5:%d, content:%x\n", content == 0xee, content);

	ptr = (uint8_t *)(0x87E0001A);
	/* write */
	*(ptr) = 0xff;
	/* read */
	content = *(ptr);
	printf("testcase10.6:%d, content:%x\n", content != 0xff, content);

	ptr = (uint8_t *)(0x87E0001E);
	/* write */
	*(ptr) = 0x99;
	/* read */
	content = *(ptr);
	printf("testcase10.7:%d, content:%x\n", content == 0x99, content);

	printf("Test 10 PASSED\n");
	jump_target();

}
void pmp_test_script() {

//	test_case10();
    muti_test();
	jump_target();
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
	uint32_t stvec = _save_context;
	uint32_t mepc = pmp_test_script;

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

	/* Config one PMP entry to enable mret */
	uint32_t pmpaddr14 = 0x00000000;
	uint32_t pmpaddr15 = addr2pmpaddr(0x87E00000);
	uint8_t pmp15cfg = 0x0f;

	write_pmpaddr(14, pmpaddr14);
	write_pmpaddr(15, pmpaddr15);
	write_pmpcfg(15, pmp15cfg);

	/* Switch to S mode and jump to kernel */
	__asm__ __volatile__(
		"mret"
	);
}
