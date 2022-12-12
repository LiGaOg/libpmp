#include "pmp_types.h"

/*
 * This function is used to read the value of pmpcfg register under RV32
 * params:
 * 	pmpcfg_id: 0~15, identify which pmpcfg you want to read.
 * return:
 * 	The value in the target pmpcfg */
uint8_t read_pmpcfg(int pmpcfg_id) {

	int shift_amount = 0;

	if (pmpcfg_id % 4 == 0) shift_amount = 0;
	if (pmpcfg_id % 4 == 1) shift_amount = 8;
	if (pmpcfg_id % 4 == 2) shift_amount = 16;
	if (pmpcfg_id % 4 == 3) shift_amount = 24;

	uint32_t pmpcfg_value;
	if (pmpcfg_id >= 0 && pmpcfg_id <= 3) {
		__asm__ __volatile__(
			"csrr %0, pmpcfg0"
			:"+r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 4 && pmpcfg_id <= 7) {
		__asm__ __volatile__(
			"csrr %0, pmpcfg1"
			:"+r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 8 && pmpcfg_id <= 11) {
		__asm__ __volatile__(
			"csrr %0, pmpcfg2"
			:"+r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 12 && pmpcfg_id <= 15) {
		__asm__ __volatile__(
			"csrr %0, pmpcfg3"
			:"+r"(pmpcfg_value)
		);
	}
	return (uint8_t)(pmpcfg_value >> shift_amount);
}

/*
 * This function is used to write pmpcfg_content to pmpcfg register under RV32
 * params:
 * 	pmpcfg_id: 0~15, identify which pmpcfg you want to write.
 * 	pmpcfg_content: Content you want to write in pmpcfg */
void write_pmpcfg(int pmpcfg_id, uint8_t pmpcfg_content) {
	
	int shift_amount = 0;
	uint32_t mask = 0x0;
	if (pmpcfg_id % 4 == 0) {
		shift_amount = 0;
		mask = 0xffffff00;
	}
	if (pmpcfg_id % 4 == 1) {
		shift_amount = 8;
		mask = 0xffff00ff;
	}
	if (pmpcfg_id % 4 == 2) {
		shift_amount = 16;
		mask = 0xff00ffff;
	}
	if (pmpcfg_id % 4 == 3) {
		shift_amount = 24;
		mask = 0x00ffffff;
	}

	uint32_t pmpcfg_value;
	if (pmpcfg_id >= 0 && pmpcfg_id <= 3) {
		__asm__ __volatile__(
			"csrr %0, pmpcfg0"
			:"+r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 4 && pmpcfg_id <= 7) {
		__asm__ __volatile__(
			"csrr %0, pmpcfg1"
			:"+r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 8 && pmpcfg_id <= 11) {
		__asm__ __volatile__(
			"csrr %0, pmpcfg2"
			:"+r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 12 && pmpcfg_id <= 15) {
		__asm__ __volatile__(
			"csrr %0, pmpcfg3"
			:"+r"(pmpcfg_value)
		);
	}
	pmpcfg_value &= mask;
	pmpcfg_value |= (pmpcfg_content << shift_amount);


	if (pmpcfg_id >= 0 && pmpcfg_id <= 3) {
		__asm__ __volatile__(
			"csrw pmpcfg0, %0"
			::"r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 4 && pmpcfg_id <= 7) {
		__asm__ __volatile__(
			"csrw pmpcfg1, %0"
			::"r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 8 && pmpcfg_id <= 11) {
		__asm__ __volatile__(
			"csrw pmpcfg2, %0"
			::"r"(pmpcfg_value)
		);
	}
	else if (pmpcfg_id >= 12 && pmpcfg_id <= 15) {
		__asm__ __volatile__(
			"csrw pmpcfg3, %0"
			::"r"(pmpcfg_value)
		);
	}
}

/*
 * This function is used to read the value of pmpaddr register under RV32
 * params:
 * 	pmpaddr_id: 0~15, identify which pmpaddr you want to read.
 * return:
 * 	The value in the target pmpaddr */
uint32_t read_pmpaddr(int pmpaddr_id) {
	uint32_t pmpaddr_value;
	if (pmpaddr_id == 0) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr0"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 1) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr1"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 2) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr2"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 3) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr3"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 4) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr4"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 5) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr5"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 6) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr6"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 7) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr7"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 8) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr8"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 9) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr9"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 10) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr10"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 11) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr11"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 12) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr12"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 13) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr13"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 14) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr14"
			:"+r"(pmpaddr_value)
		);
	}
	else if (pmpaddr_id == 15) {
		__asm__ __volatile__(
			"csrr %0, pmpaddr15"
			:"+r"(pmpaddr_value)
		);
	}
	return pmpaddr_value;
}

/*
 * This function is used to write pmpaddr_content to pmpaddr register under RV32
 * params:
 * 	pmpaddr_id: 0~15, identify which pmpaddr you want to write.
 * 	pmpaddr_content: Content you want to write in pmpaddr */
void write_pmpaddr(int pmpaddr_id, uint32_t pmpaddr_content) {
	if (pmpaddr_id == 0) {
		__asm__ __volatile__(
			"csrw pmpaddr0 %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 1) {
		__asm__ __volatile__(
			"csrw pmpaddr1, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 2) {
		__asm__ __volatile__(
			"csrw pmpaddr2, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 3) {
		__asm__ __volatile__(
			"csrw pmpaddr3, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 4) {
		__asm__ __volatile__(
			"csrw pmpaddr4, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 5) {
		__asm__ __volatile__(
			"csrw pmpaddr5, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 6) {
		__asm__ __volatile__(
			"csrw pmpaddr6, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 7) {
		__asm__ __volatile__(
			"csrw pmpaddr7, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 8) {
		__asm__ __volatile__(
			"csrw pmpaddr8, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 9) {
		__asm__ __volatile__(
			"csrw pmpaddr9, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 10) {
		__asm__ __volatile__(
			"csrw pmpaddr10, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 11) {
		__asm__ __volatile__(
			"csrw pmpaddr11, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 12) {
		__asm__ __volatile__(
			"csrw pmpaddr12, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 13) {
		__asm__ __volatile__(
			"csrw pmpaddr13, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 14) {
		__asm__ __volatile__(
			"csrw pmpaddr14, %0"
			::"r"(pmpaddr_content)
		);
	}
	else if (pmpaddr_id == 15) {
		__asm__ __volatile__(
			"csrw pmpaddr15, %0"
			::"r"(pmpaddr_content)
		);
	}
}
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
/* Refresh middle layer to physical PMP entries */
void refresh() {

	/* Trigger breakpoint exception and jump to M mode */
	__asm__ __volatile__(
		"ebreak"
	);
}
