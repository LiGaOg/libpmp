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

Node* collect_intersect_nodes(unsigned int start, unsigned int end){
  Node* head = NULL;
  Node* insert_temp = NULL;
  Node* temp = pmp_node_head;
  while(temp!= NULL){
    Node* next = temp->next;
    if(next == NULL){
      break;
    }

    int next_start = next -> start, next_end = next-> end;
    if(next_start <= end && start <= next_end){
      Node* next_next = next -> next;
      if(head == NULL){
        head = next;
        insert_temp = next;
      }else{
        insert_temp -> next = next;
        insert_temp = insert_temp -> next;
      }
      temp -> next = next_next;
    }else {
      temp = temp -> next;
    }
  }

  return head;
}

int contains(int* arr,int arr_length, int val){
  for(int i = 0; i < arr_length; i++){
    if(arr[i] == val){
      return 1;
    }
  }

  return 0;
}

void sort_arr(int* arr, int arr_length){
  for (int i = 0; i < arr_length - 1; i++) {
    for (int j = 0; j < arr_length - 1 - i; j++) {
      if(arr[j] > arr[j + 1]){
        int temp = arr[j];
        arr[j] = arr[j + 1];
        arr[j + 1] = temp;
      }
    }
  }
}

Node* reorganize_collected(Node* collected, 
          unsigned int start, unsigned int end, 
          unsigned char privilege, int v_pmp_id){
  Node* head = collected;

  int all_place[1000];
  int number = 0;

  all_place[number] = start;
  number++;
  all_place[number] = end;
  number++;

  while(collected != NULL){
    int s = collected -> start, e = collected -> end;
    if(!contains(all_place, number, s)){
      all_place[number] = s;
      number++;
    }

    if(!contains(all_place, number, e)){
      all_place[number] = e;
      number++;
    }

    collected = collected -> next;
  }

  sort_arr(all_place, number);
  
  Node* temp = head;
  Node* result_head = NULL;
  Node* result_temp = NULL;

  int all_place_index = 0;
  while(all_place[all_place_index] < start){
    all_place_index++;
  }

  while(all_place[all_place_index] < end){
    int new_start = all_place[all_place_index];
    int new_end = all_place[all_place_index + 1];
    Node* new_node = malloc(sizeof(Node));
    new_node -> start = new_start;
    new_node -> end = new_end;
    new_node -> privilege = privilege;
    new_node -> flag = 0;
    new_node -> next = NULL;
    new_node -> v_pmp_id = v_pmp_id;

    all_place_index++;
    if(result_head == NULL){
      result_head = new_node;
      result_temp = new_node;
    }else{
      result_temp -> next = new_node;
      result_temp = result_temp->next;
    }
  }

  while(temp != NULL){
    all_place_index = 0;
    while(all_place[all_place_index] < temp -> start){
      all_place_index++;
    }

    while(all_place[all_place_index] < temp -> end){
      int new_start = all_place[all_place_index];
      int new_end = all_place[all_place_index + 1];
      Node* new_node = malloc(sizeof(Node));
      new_node -> start = new_start;
      new_node -> end = new_end;
      new_node -> privilege = temp -> privilege;
      new_node -> flag = 0;
      new_node -> next = NULL;
      new_node -> v_pmp_id = temp -> v_pmp_id;

      all_place_index++;
      if(result_head == NULL){
        result_head = new_node;
        result_temp = new_node;
      }else{
        result_temp -> next = new_node;
        result_temp = result_temp->next;
      }
    }
    Node* tobe_free = temp;

    temp = temp -> next;
    free(tobe_free);
  }

  return result_head;
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
  Node* collected = collect_intersect_nodes(start, end);
  Node* reorganized = reorganize_collected(collected, start, end, privilege, v_pmp_id);

  Node* temp = pmp_node_head;

  while(temp -> next != NULL){
    temp = temp -> next;
  }

  temp -> next = reorganized;

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
