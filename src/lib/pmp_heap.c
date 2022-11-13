#include "pmp_heap.h"

Node* pmp_node_head;
int pmp_cnt;

int pmp_init(){
    pmp_cnt = 0;                                        // set to 0, means that no pmp_entry yet
    pmp_node_head = (Node *)malloc(sizeof(Node));       // use head to control whole list
    pmp_node_head->next = NULL;
    pmp_node_head->v_pmp_id = -1;                       // set head pmp_id to -1
    return 0;
}

void insert(Node* node){
    Node* temp = pmp_node_head;

    while(temp -> next != NULL){
        temp = temp -> next;
    }

    temp -> next = node;
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
        /* asm volatile( */
        /*         "mv pmpcfg0, %0;\ */
        /*         mv pmpcfg1, %1;\ */
        /*         mv pmpcfg2, %2;\ */
        /*         mv pmpcfg3, %3;\ */
        /*         mv pmpaddr0, %4;\ */
        /*         mv pmpaddr1, %5;\ */
        /*         mv pmpaddr2, %6;\ */
        /*         mv pmpaddr3, %7;\ */
        /*         mv pmpaddr4, %8;\ */
        /*         mv pmpaddr5, %9;\ */
        /*         mv pmpaddr6, %10;\ */
        /*         mv pmpaddr7, %11;\ */
        /*         mv pmpaddr8, %12;\ */
        /*         mv pmpaddr9, %13;\ */
        /*         mv pmpaddr10, %14;\ */
        /*         mv pmpaddr11, %15;\ */
        /*         mv pmpaddr12, %16;\ */
        /*         mv pmpaddr13, %17;\ */
        /*         mv pmpaddr14, %18;\ */
        /*         mv pmpaddr15, %19;" */
        /*         : */
        /*         :"r"(pmpcfg[0]),    // %0 */
        /*         "r"(pmpcfg[1]),     // %1 */
        /*         "r"(pmpcfg[2]),     // %2 */
        /*         "r"(pmpcfg[3]),     // %3 */
        /*         "r"(pmpaddr[0]),    // %4 */
        /*         "r"(pmpaddr[1]),    // %5 */
        /*         "r"(pmpaddr[2]),    // %6 */
        /*         "r"(pmpaddr[3]),    // %7 */
        /*         "r"(pmpaddr[4]),    // %8 */
        /*         "r"(pmpaddr[5]),    // %9 */
        /*         "r"(pmpaddr[6]),    // %10 */
        /*         "r"(pmpaddr[7]),    // %11 */
        /*         "r"(pmpaddr[8]),    // %12 */
        /*         "r"(pmpaddr[9]),    // %13 */
        /*         "r"(pmpaddr[10]),   // %14 */
        /*         "r"(pmpaddr[11]),   // %15 */
        /*         "r"(pmpaddr[12]),   // %16 */
        /*         "r"(pmpaddr[13]),   // %17 */
        /*         "r"(pmpaddr[14]),   // %18 */
        /*         "r"(pmpaddr[15])    // %19 */
        /*         : */
    /* ); */
}
