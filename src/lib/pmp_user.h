#include "pmp_heap.h"
unsigned char query_privilege(unsigned int addr);

void pmp_mmap(unsigned int start, unsigned int end, unsigned char privilege, int v_pmp_id);

void logNode(Node* node);

void log();