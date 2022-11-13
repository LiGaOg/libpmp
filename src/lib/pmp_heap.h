#ifndef __PMP_HEAP_H__
#define __PMP_HEAP_H__

#include <stdlib.h>
#include <stdio.h>

#define cache_len 8


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

typedef struct Node Node;

struct Node{
    unsigned int start;
    unsigned int end;
    uint8_t privilege;
    Node* next;
    int flag; // whether it is in PMP entry;
    int v_pmp_id; // 0-9999
};

//MALLOC

//lower layer: pmp entry
//middle layer: 8 interval cache
//higher layer: 100 interval

extern Node* pmp_node_head;
extern int pmp_cnt;

extern Node* cache[cache_len]; // middle layer

int pmp_init();

void insert(Node* node);
void pmp_free(int v_pmp_id);

// middle layer to lower layer
void refresh();


#endif
