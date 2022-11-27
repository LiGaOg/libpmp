#include "pmp_heap.h"

Node* cache[cache_len]; // middle layer
			       //
Node* pmp_node_head;
int pmp_cnt;
long long system_time;

int pmp_init(){
    pmp_cnt = 0;                                        // set to 0, means that no pmp_entry yet
    pmp_node_head = (Node *)malloc(sizeof(Node));       // use head to control whole list
    pmp_node_head->next = NULL;
    pmp_node_head->v_pmp_id = -1; 
    for(int i = 0 ; i< cache_len; i++){
        cache[i] = NULL;
    }                      // set head pmp_id to -1
    system_time = 0;
    return 0;
}

void insert(Node* node){
    Node* temp = pmp_node_head;

    while(temp -> next != NULL){
        temp = temp -> next;
    }

    temp -> next = node;
    pmp_cnt += 1;
}

void pmp_free(int v_pmp_id){
    int in_PMP_entry = 0;
    Node *index = pmp_node_head;
    Node *lastIndex = NULL;
    while(index != NULL){
        if(index->v_pmp_id == v_pmp_id){
            in_PMP_entry = 1;
            lastIndex->next = index->next;
            break;
        }else{
            lastIndex = index;
            index = index->next;
        }
    }
    // if it's in cache[8], then need to remove and refresh to real pmp_entry
    if(in_PMP_entry == 1){
        int idx = -1;
        for(int i = 0; i < cache_len; i++){
            if(cache[i]->v_pmp_id == v_pmp_id){
                idx = i;
                break;
            }
        }
        if(idx != cache_len - 1){
            for(int i = idx + 1; i < cache_len; i++){
                cache[i - 1] = cache[i];
            }
        }
        cache[cache_len - 1] = NULL;
        refresh();
    }
    // finally, del the whole struct
   free(index);
}

void sort_cache(){
    // use the easiest bumble sort
    for(int i = 0; i < cache_len - 1; i++){
        for(int j = 0; j < cache_len - i - 1; j++){
            int v_pmp_id_j = 10000;
            int v_pmp_id_j_add_1 = 10000;
            if(cache[j] != NULL)v_pmp_id_j = cache[j]->v_pmp_id;
            if(cache[j + 1] != NULL)v_pmp_id_j_add_1 = cache[j + 1]->v_pmp_id;
            if(v_pmp_id_j> v_pmp_id_j_add_1){
                Node* tmp = cache[j];
                cache[j] = cache[j + 1];
                cache[j + 1] = tmp;
            }
        }
    }
}

void refresh(){
    sort_cache();
    uint32_t pmpaddr[16];
    uint32_t pmpcfg[4];
    for(int i = cache_len - 1; i >= 0; i--){
        if(cache[i] != NULL){
            pmpcfg[i / 2] = pmpcfg[i / 2] << 8;
            pmpcfg[i / 2] = pmpcfg[i / 2] | 1;
            pmpcfg[i / 2] = pmpcfg[i / 2] << 8;
            pmpaddr[i * 2] = cache[i]->start;
            pmpaddr[i * 2 + 1] = cache[i]->end;
        }else{
            pmpcfg[i / 2] = pmpcfg[i / 2] << 8;
            pmpcfg[i / 2] = pmpcfg[i / 2] << 8;
            pmpaddr[i * 2] = 0;
            pmpaddr[i * 2 + 1] = 0;
        }
    }
    // assembly
        asm volatile(
                "csrw pmpcfg0, %0;\
                csrw pmpcfg1, %1;\
                csrw pmpcfg2, %2;\
                csrw pmpcfg3, %3;\
                csrw pmpaddr0, %4;\
                csrw pmpaddr1, %5;\
                csrw pmpaddr2, %6;\
                csrw pmpaddr3, %7;\
                csrw pmpaddr4, %8;\
                csrw pmpaddr5, %9;\
                csrw pmpaddr6, %10;\
                csrw pmpaddr7, %11;\
                csrw pmpaddr8, %12;\
                csrw pmpaddr9, %13;\
                csrw pmpaddr10, %14;\
                csrw pmpaddr11, %15;\
                csrw pmpaddr12, %16;\
                csrw pmpaddr13, %17;\
                csrw pmpaddr14, %18;\
                csrw pmpaddr15, %19;"
                :
                :"r"(pmpcfg[0]),    // %0
                "r"(pmpcfg[1]),     // %1
                "r"(pmpcfg[2]),     // %2
                "r"(pmpcfg[3]),     // %3
                "r"(pmpaddr[0]),    // %4
                "r"(pmpaddr[1]),    // %5
                "r"(pmpaddr[2]),    // %6
                "r"(pmpaddr[3]),    // %7
                "r"(pmpaddr[4]),    // %8
                "r"(pmpaddr[5]),    // %9
                "r"(pmpaddr[6]),    // %10
                "r"(pmpaddr[7]),    // %11
                "r"(pmpaddr[8]),    // %12
                "r"(pmpaddr[9]),    // %13
                "r"(pmpaddr[10]),   // %14
                "r"(pmpaddr[11]),   // %15
                "r"(pmpaddr[12]),   // %16
                "r"(pmpaddr[13]),   // %17
                "r"(pmpaddr[14]),   // %18
                "r"(pmpaddr[15])    // %19
                :
    );
    for(int i = 0; i < 16; i++){
	    printf("pmpaddr%d, %x\n", i, pmpaddr[i]);
    }
    unsigned int mask = 0x000000ff;
    for(int i = 0; i < 4; i++){
	for (int j = 0; j < 4; j++) {
		printf("pmp%dcfg, %x\n", i * 4 + j, (pmpcfg[i] >> 8 * j) & mask);
	}
    }
}
