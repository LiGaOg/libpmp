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
