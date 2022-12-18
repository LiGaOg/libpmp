#include "pmp_types.h"
#include "pmp_system_library.h"
/* Head of virtual pmp entries */
virtual_pmp_entry_head dummy = { 0, NULL };

/* Judge whether two virtual pmp entry are equal */
int is_equal(virtual_pmp_entry *v1, virtual_pmp_entry *v2) {
	return (v1->start == v2->start && v1->end == v2->end && \
			v1->priority == v2->priority && v1->privilege == v2->privilege);
}
/* Delete target virtual_pmp_entry in LinkedList */
void delete_virtual_pmp_entry(virtual_pmp_entry *target) {
	virtual_pmp_entry *cur = dummy.head;
	int is_delete_head = 0;
	for (int i = 0; i < dummy.number_of_node; i ++) {
		if (is_equal(cur, target)) {
			if (target->prev == NULL) {
				is_delete_head = 1;
				continue;
			}
			if (target->prev != NULL)
				target->prev->next = target->next;
			/* If this is not the last node */
			if (target->next != NULL) target->next->prev = target->prev;
			dummy.number_of_node --;
		}
	}
	if (is_delete_head) {
		if (target->next != NULL)
			target->next->prev = NULL;
		dummy.head = target->next;
		dummy.number_of_node --;
	}
	if (dummy.number_of_node == 0) dummy.head = NULL;
}

/* Add one virtual_pmp_entry in LinkedList */
void add_virtual_pmp_entry(virtual_pmp_entry *target) {
	/* If this linkedlist is empty */
	if (dummy.number_of_node == 0) {
		dummy.number_of_node ++;
		dummy.head = target;
		target->prev = NULL;
		target->next = NULL;
	}
	else  {
		dummy.number_of_node ++;
		target->prev = NULL;
		target->next = dummy.head;
		dummy.head->prev = target;
		dummy.head = target;
	}
}

middle_layer * middle;

/* Initialize middle layer structure */
void init_middle_layer() {
	middle = (middle_layer *)malloc(sizeof(middle_layer));
	middle->number_of_node = 0;
	for (int i = 0; i < CACHE_LEN; i ++) {
		middle->cache[i] = NULL;
	}
}

/* Return the index of virtual pmp entry to be evicted */
int evict_from_middle() {
	return 0;
}

/* This function adjust middle layer according to priority of
 * virtual pmp entries */
void adjust_middle_layer() {
	/* Split NULL region */
	int i = 0, j = 6;
	while (i < j) {
		while (middle->cache[i] != NULL) i ++;
		while (middle->cache[j] == NULL) j --;
		if (i < j) {
			middle->cache[i] = middle->cache[j];
			middle->cache[j] = NULL;
		}
	}
	/* Now middle->cache[0, i-1] stores virtual_pmp_entry, sort according to priority */
	/* Sorting algorithm is bubble sort */
	for (int u = 0; u < i; u ++) {
		for (int v = u + 1; v < i; v ++) {
			if (middle->cache[u]->priority > middle->cache[v]->priority) {
				virtual_pmp_entry * tmp = middle->cache[u];
				middle->cache[u] = middle->cache[v];
				middle->cache[v] =  tmp;
			}
		}
	}
}

/* This function add one virtual pmp entries into the middle layer
 * If the middle layer is full, it will evict one virtual pmp entry 
 * in the layer and put it to the linkedlist */
void add_virtual_pmp_entry_to_cache(virtual_pmp_entry *target) {
	/* If cache is not empty */
	if (middle->number_of_node < CACHE_LEN) {
		middle->cache[middle->number_of_node] = target;
		adjust_middle_layer();
		middle->number_of_node ++;
	}
}


/* This function is used to delete virtual pmp entry in middle layer */
void delete_middle_layer_entry(virtual_pmp_entry *target) {
	for (int i = 0; i < middle->number_of_node; i ++) {
		if (is_equal(middle->cache[i], target)) {
			middle->cache[i] = NULL;
			middle->number_of_node --;
		}
	}
	adjust_middle_layer();
}

