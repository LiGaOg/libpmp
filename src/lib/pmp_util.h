#ifndef __PMP_UTIL_H__
#define __PMP_UTIL_H__
#include "pmp_types.h"
#include "pmp_system_library.h"
/*
 * This function is used to read the value of pmpcfg register under RV32
 * params:
 * 	pmpcfg_id: 0~15, identify which pmpcfg you want to read.
 * return:
 * 	The value in the target pmpcfg */
extern uint8_t read_pmpcfg(int pmpcfg_id); 


/*
 * This function is used to write pmpcfg_content to pmpcfg register under RV32
 * params:
 * 	pmpcfg_id: 0~15, identify which pmpcfg you want to write.
 * 	pmpcfg_content: Content you want to write in pmpcfg */
extern void write_pmpcfg(int pmpcfg_id, uint8_t pmpcfg_content);

/*
 * This function is used to read the value of pmpaddr register under RV32
 * params:
 * 	pmpaddr_id: 0~15, identify which pmpaddr you want to read.
 * return:
 * 	The value in the target pmpaddr */
extern uint32_t read_pmpaddr(int pmpaddr_id);

/*
 * This function is used to write pmpaddr_content to pmpaddr register under RV32
 * params:
 * 	pmpaddr_id: 0~15, identify which pmpaddr you want to write.
 * 	pmpaddr_content: Content you want to write in pmpaddr */
extern void write_pmpaddr(int pmpaddr_id, uint32_t pmpaddr_content);

extern uint32_t addr2pmpaddr(uint32_t addr);
/*
 * This function is used to find the entry which contains addr
 * and has the highest priority
 * param:
 * 	addr: target address
 * return:
 * 	target entry, return NULL if no virtual pmp entry contain
 * 	that addr */
extern virtual_pmp_entry *find_highest_priority_entry(uint32_t addr);
/* Refresh middle layer to physical PMP entries */
extern void refresh(); 


extern uint32_t max(uint32_t s1, uint32_t s2);
extern uint32_t min(uint32_t s1, uint32_t s2); 

extern int is_intersect(uint32_t s1, uint32_t e1, uint32_t s2, uint32_t e2); 

extern void virtual_pmp_entry_free(virtual_pmp_entry *head); 
#endif
