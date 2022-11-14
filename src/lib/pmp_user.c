#include "pmp_user.h"

int first_empty(){
    for(int i = 0; i < cache_len; i++){
        if(cache[i] == NULL){
            return i;
        }
    }
    return -1;
}

int cache_full(){
    return first_empty() == -1;
}

unsigned char query_privilege(unsigned int addr){
    Node* highest_priority_fit = NULL;
    unsigned char default_privilege = 0;
    
    Node* temp = pmp_node_head;
    while(temp != NULL){
        if(temp -> start >= addr && temp -> end < addr){
            // address fit
            if(highest_priority_fit == NULL
                || highest_priority_fit->v_pmp_id >= temp->v_pmp_id){
                    highest_priority_fit = temp;
                }
        }
        temp = temp -> next;
    }

    if(highest_priority_fit == NULL){
        // not fit pmp virtual entry, just return the default privilege
        // What's the default privilege??
        // According to the book of riscv, the default privilege is all-0, just reject in s and u.
        // This will trigger exception/interrupt.
        
        return default_privilege;
    }else{
        if(highest_priority_fit -> flag){
            // it is in pmp entry, no updating
            return highest_priority_fit -> privilege;
        }else {
            
            //case 1: cache is full
            if(cache_full()){
                int evictIndex = 0; 
                cache[evictIndex] -> flag = 0;
                cache[evictIndex] = highest_priority_fit;
                // evict the original index from cache
            }else{ // case 2: cache is not full
                int index = first_empty();
                cache[index] = highest_priority_fit;
                // just put the cache into the first empty index
            }

            highest_priority_fit -> flag = 1;
            // let the selected one with highest priority into the cache
            refresh();

            return highest_priority_fit->privilege;

        }
    }
}

void pmp_mmap(unsigned int start, unsigned int end, unsigned char privilege, int v_pmp_id){
    Node *request = (Node *)malloc(sizeof(Node));
    request->start = start;
    request->end = end;
    request->privilege = privilege;
    request->v_pmp_id = v_pmp_id;
    request->flag = 0;

    // TODO : whether or not need to judget the List is created
    insert(request);
}

void logNode(Node* node){
    printf("[Node] start = %x, end = %x, privilege = %x, flag = %d, v_pmp_id = %d"
        , node -> start, node -> end, node -> privilege, node -> flag, node -> v_pmp_id);
}

void log(){
    printf("[High level] pmp_cnt = %d\n", pmp_cnt);
    
    Node* node = pmp_node_head;
    while(node != NULL){
        logNode(node);
        node = node -> next;
        printf("\n");
    }
    
    printf("[Mid level]\n");
    for(int i =0; i < 8; i++){
        if(cache[i] == NULL){
            printf("cache[%d]: NULL\n", i);
        }else{
            printf("cache[%d]: ", i);
            logNode(cache[i]);
            printf("\n");
        }
    }
    
}