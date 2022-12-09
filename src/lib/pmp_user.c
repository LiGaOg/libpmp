#include "pmp_user.h"

int first_empty() {
  for (int i = 0; i < cache_len; i++) {
    if (cache[i] == NULL) {
      return i;
    }
  }
  return -1;
}

int cache_full() { return first_empty() == -1; }

int lru_evict_index() {
  int index = 0;
  for (int i = 0; i < cache_len; i++) {
    if (cache[i]->lru_time <= cache[index]->lru_time) {
      index = i;
    }
  }

  return index;
}

unsigned char query_privilege(unsigned int addr) {
  system_time++;
  Node *highest_priority_fit = NULL;
  unsigned char default_privilege = 0;

  Node *temp = pmp_node_head;
  while (temp != NULL) {
    if (temp->start <= addr && temp->end > addr) {
      // address fit
      if (highest_priority_fit == NULL ||
          highest_priority_fit->v_pmp_id >= temp->v_pmp_id) {
        highest_priority_fit = temp;
      }
    }
    temp = temp->next;
  }

  if (highest_priority_fit == NULL) {
    // not fit pmp virtual entry, just return the default privilege
    // What's the default privilege??
    // According to the book of riscv, the default privilege is all-0, just
    // reject in s and u. This will trigger exception/interrupt.

    return default_privilege;
  } else {
    highest_priority_fit->lru_time = system_time;
    if (highest_priority_fit->flag) {
      // it is in pmp entry, no updating
      return highest_priority_fit->privilege;
    } else {
      // case 1: cache is full
      if (cache_full()) {
        // use LRU algorithm to evict one entry
        int evictIndex = lru_evict_index();
        cache[evictIndex]->flag = 0;
        cache[evictIndex] = highest_priority_fit;
        // evict the original index from cache
      } else { // case 2: cache is not full
        int index = first_empty();
        cache[index] = highest_priority_fit;
        // just put the cache into the first empty index
      }

      highest_priority_fit->flag = 1;
      // let the selected one with highest priority into the cache
      refresh();

      return highest_priority_fit->privilege;
    }
  }
}

Node* create_4nodes(Node* tobe_freed_next, unsigned int start, unsigned int end, unsigned char privilege, int v_pmp_id){
  unsigned int next_start = tobe_freed_next->start, next_end = tobe_freed_next-> end;
  unsigned char next_privilege = tobe_freed_next->privilege;
  int next_v_pmp_id = tobe_freed_next -> v_pmp_id;

  if(start < next_start){
    if(end < next_end){
      Node* node1 = malloc(sizeof(Node));
      node1 -> start = start;
      node1 -> end = next_start;
      node1 -> privilege = privilege;
      node1 -> flag = 0;
      node1 -> v_pmp_id = v_pmp_id;
      node1 -> lru_time = 0;

      Node* node2 = malloc(sizeof(Node));
      node2 -> start = next_start;
      node2 -> end = end;
      node2 -> privilege = privilege;
      node2 -> flag = 0;
      node2 -> v_pmp_id = v_pmp_id;
      node2 -> lru_time = 0;

      Node* node3 = malloc(sizeof(Node));
      node3 -> start = next_start;
      node3 -> end = end;
      node3 -> privilege = next_privilege;
      node3 -> flag = 0;
      node3 -> v_pmp_id = next_v_pmp_id;
      node3 -> lru_time = 0;

      Node* node4 = malloc(sizeof(Node));
      node4 -> start = next_start;
      node4 -> end = end;
      node4 -> privilege = next_privilege;
      node4 -> flag = 0;
      node4 -> v_pmp_id = next_v_pmp_id;
      node4 -> lru_time = 0;

      node1 -> next = node2;
      node2 -> next = node3;
      node3 -> next = node4;
      node4 -> next = NULL;
      return node1;
    }else {
      Node* node1 = malloc(sizeof(Node));
      node1 -> start = start;
      node1 -> end = next_start;
      node1 -> privilege = privilege;
      node1 -> flag = 0;
      node1 -> v_pmp_id = v_pmp_id;
      node1 -> lru_time = 0;

      Node* node2 = malloc(sizeof(Node));
      node2 -> start = next_start;
      node2 -> end = next_end;
      node2 -> privilege = privilege;
      node2 -> flag = 0;
      node2 -> v_pmp_id = v_pmp_id;
      node2 -> lru_time = 0;

      Node* node3 = malloc(sizeof(Node));
      node3 -> start = next_end;
      node3 -> end = end;
      node3 -> privilege = privilege;
      node3 -> flag = 0;
      node3 -> v_pmp_id = v_pmp_id;
      node3 -> lru_time = 0;

      Node* node4 = malloc(sizeof(Node));
      node4 -> start = next_start;
      node4 -> end = next_end;
      node4 -> privilege = next_privilege;
      node4 -> flag = 0;
      node4 -> v_pmp_id = next_v_pmp_id;
      node4 -> lru_time = 0;

      node1 -> next = node2;
      node2 -> next = node3;
      node3 -> next = node4;
      node4 -> next = NULL;
      return node1;
    }
  }else {
    if(end < next_end){
      Node* node1 = malloc(sizeof(Node));
      node1 -> start = next_start;
      node1 -> end = start;
      node1 -> privilege = next_privilege;
      node1 -> flag = 0;
      node1 -> v_pmp_id = next_v_pmp_id;
      node1 -> lru_time = 0;

      Node* node2 = malloc(sizeof(Node));
      node2 -> start = start;
      node2 -> end = end;
      node2 -> privilege = next_privilege;
      node2 -> flag = 0;
      node2 -> v_pmp_id = next_v_pmp_id;
      node2 -> lru_time = 0;

      Node* node3 = malloc(sizeof(Node));
      node3 -> start = end;
      node3 -> end = next_end;
      node3 -> privilege = next_privilege;
      node3 -> flag = 0;
      node3 -> v_pmp_id = next_v_pmp_id;
      node3 -> lru_time = 0;

      Node* node4 = malloc(sizeof(Node));
      node4 -> start = start;
      node4 -> end = end;
      node4 -> privilege = privilege;
      node4 -> flag = 0;
      node4 -> v_pmp_id = v_pmp_id;
      node4 -> lru_time = 0;

      node1 -> next = node2;
      node2 -> next = node3;
      node3 -> next = node4;
      node4 -> next = NULL;
      return node1;
    }else{
      Node* node1 = malloc(sizeof(Node));
      node1 -> start = next_start;
      node1 -> end = start;
      node1 -> privilege = next_privilege;
      node1 -> flag = 0;
      node1 -> v_pmp_id = next_v_pmp_id;
      node1 -> lru_time = 0;

      Node* node2 = malloc(sizeof(Node));
      node2 -> start = start;
      node2 -> end = next_end;
      node2 -> privilege = next_privilege;
      node2 -> flag = 0;
      node2 -> v_pmp_id = next_v_pmp_id;
      node2 -> lru_time = 0;

      Node* node3 = malloc(sizeof(Node));
      node3 -> start = start;
      node3 -> end = next_end;
      node3 -> privilege = privilege;
      node3 -> flag = 0;
      node3 -> v_pmp_id = v_pmp_id;
      node3 -> lru_time = 0;

      Node* node4 = malloc(sizeof(Node));
      node4 -> start = next_end;
      node4 -> end = end;
      node4 -> privilege = privilege;
      node4 -> flag = 0;
      node4 -> v_pmp_id = v_pmp_id;
      node4 -> lru_time = 0;

      node1 -> next = node2;
      node2 -> next = node3;
      node3 -> next = node4;
      node4 -> next = NULL;
      return node1;
    }
  }
}

Node* collect_intersect_nodes(unsigned int start, unsigned int end, unsigned char privilege, int v_pmp_id){
  Node* head = NULL;
  Node* add_temp = NULL;
  Node* temp = pmp_node_head;

  int has_intersect = 0;
  
  while(temp != NULL){
    Node* next = temp -> next;
    if(next == NULL){
      break;
    }

    unsigned int next_start = next->start, next_end = next-> end;
    unsigned char next_privilege = next->privilege;
    int next_v_pmp_id = next -> v_pmp_id;

    int intersect = next_start < end || start < next_end;
    if(intersect){
      has_intersect = 1;
      Node* next_next = next->next;
      temp -> next = next_next;

      Node* created_4nodes = create_4nodes(next, start, end, privilege, v_pmp_id);
      if(head == NULL){
        head = created_4nodes;
      }else{
        add_temp = head;
        while(1){
          if(add_temp -> next == NULL){
            add_temp -> next = created_4nodes;
            break;
          }
          add_temp = add_temp->next;
        }
      }
      free(next);
    }
    
    temp = temp -> next;
  }

  if(has_intersect){
    return head;
  }else{
    head = malloc(sizeof(Node));
    head -> start = start;
    head -> end = end;
    head -> privilege = privilege;
    head -> v_pmp_id = v_pmp_id;
    return head;
  }
}

void clear_cache(){
  for(int i = 0; i < cache_len; i++){
    if(cache[i] != NULL){
      cache[i] -> flag = 0;
      cache[i] = NULL;
    }
  }
}

void set_cache(unsigned int start){
  int index = 0;
  while(index < cache_len){
    int hit = 0;

    Node* temp = pmp_node_head;
    Node* highest_priority_fit = NULL;
    while(temp != NULL){
      if(temp -> start == start){
        hit = 1;
        if(highest_priority_fit == NULL || highest_priority_fit->v_pmp_id >= temp -> v_pmp_id){
          highest_priority_fit = temp;
        }
      }

      temp = temp->next;
    }

    if(!hit){
      break;
    }

    cache[index] = highest_priority_fit;
    cache[index] -> flag = 1;
    index++;
    start = highest_priority_fit->end;
  }
}

void pmp_mmap(unsigned int start, unsigned int end, unsigned char privilege,
              int v_pmp_id) {
  clear_cache();
  Node* collected = collect_intersect_nodes(start, end, privilege, v_pmp_id);

  Node* temp = pmp_node_head;

  while(temp -> next != NULL){
    temp = temp -> next;
  }

  temp -> next = collected;

  set_cache(start);
  refresh();

  // Node *request = (Node *)malloc(sizeof(Node));
  // request->start = start;
  // request->end = end;
  // request->privilege = privilege;
  // request->v_pmp_id = v_pmp_id;
  // request->flag = 0;
  // request->lru_time = 0;

  // TODO : whether or not need to judget the List is created
  // insert(request);
}

void logNode(Node *node) {
  printf(
      "[Node] start = %x, end = %x, privilege = %x, flag = %d, v_pmp_id = %d",
      node->start, node->end, node->privilege, node->flag, node->v_pmp_id);
}

void log() {
  printf("[High level] pmp_cnt = %d\n", pmp_cnt);

  Node *node = pmp_node_head;
  while (node != NULL) {
    logNode(node);
    node = node->next;
    printf("\n");
  }

  printf("[Mid level]\n");
  for (int i = 0; i < 8; i++) {
    if (cache[i] == NULL) {
      printf("cache[%d]: NULL\n", i);
    } else {
      printf("cache[%d]: ", i);
      logNode(cache[i]);
      printf("\n");
    }
  }
}
