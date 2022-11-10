#include "pmp_heap.h"
#include "pmp_user.h"

void pmp_mmap(unsigned int start, unsigned int end, unsigned char privilege, int v_pmp_id){
    Node *request = (Node *)malloc(sizeof(Node));
    request->start = start;
    request->end = end;
    request->privilege = privilege;
    request->v_pmp_id = v_pmp_id;
    // TODO : whether or not need to judget the List is created
    insert(request);
}
