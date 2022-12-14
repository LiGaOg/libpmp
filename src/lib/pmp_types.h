#ifndef __PMP_TYPES_H__
#define __PMP_TYPES_H__
/* Integer types */

#define NULL 0
#define XLEN 32
#define MXLEN 32
#define CACHE_LEN 7
typedef unsigned int size_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

/* LinkedList */
struct virtual_pmp_entry {

	struct virtual_pmp_entry *prev;
  	struct virtual_pmp_entry *next;
	/* start address of this virtual pmp entry */
	uint32_t start;
	/* end address of this virtual pmp entry */
  	uint32_t end;
	/* Privilege of this range, the meaning of this value
	 * is the same as the manual */
  	uint8_t privilege;
	/* The static priority of PMP entry, more less this
	 * variable is, more prior this entry is. 
	 * Currently support 10000 virtual pmp entries */
  	int priority;
};
typedef struct virtual_pmp_entry virtual_pmp_entry;


/* Def of head node of virtual pmp entries */
struct virtual_pmp_entry_head {

	/* Number of node in LinkedList */
	int number_of_node;
	/* Head node */
	virtual_pmp_entry *head;
};
typedef struct virtual_pmp_entry_head virtual_pmp_entry_head;


/* Middle layer */
struct middle_layer {
	
	/* Number of virtual pmp entries that are in middle layer */
	int number_of_node;
	/* Actual pmp entry, the pmp entry is sorted with priority */
	virtual_pmp_entry * cache[CACHE_LEN];
};
typedef struct middle_layer middle_layer;

/* Define several important variables and member functions */

/* Head of virtual pmp entries */
extern virtual_pmp_entry_head dummy;

/* Judge whether two virtual pmp entry are equal */
extern int is_equal(virtual_pmp_entry *v1, virtual_pmp_entry *v2);

/* Delete target virtual_pmp_entry in LinkedList */
extern void delete_virtual_pmp_entry(virtual_pmp_entry *target);

/* Add one virtual_pmp_entry in LinkedList */
extern void add_virtual_pmp_entry(virtual_pmp_entry *target);

extern middle_layer * middle;

/* Initialize middle layer structure */
extern void init_middle_layer();

/* Return the index of virtual pmp entry to be evicted */
extern int evict_from_middle();

/* This function adjust middle layer according to priority of
 * virtual pmp entries */
extern void adjust_middle_layer();

/* This function add one virtual pmp entries into the middle layer
 * If the middle layer is full, it will evict one virtual pmp entry 
 * in the layer and put it to the linkedlist */
extern void add_virtual_pmp_entry_to_cache(virtual_pmp_entry *target); 


/* This function is used to delete virtual pmp entry in middle layer */
extern void delete_middle_layer_entry(virtual_pmp_entry *target);


#endif
