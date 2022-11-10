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
    // if it's in cache[16], then need to remove and refresh to real pmp_entry
    if(in_PMP_entry == 1){
        int idx = -1;
        for(int i = 0; i < 16; i++){
            if(cache[i]->v_pmp_id == v_pmp_id){
                idx = i;
                break;
            }
        }
        if(idx != 15){
            for(int i = idx + 1; i < 16; i++){
                cache[i - 1] = cache[i];
            }
        }
        cache[15] = NULL;
        refresh();
    }
    // finally, del the whole struct
   free(index);
}
