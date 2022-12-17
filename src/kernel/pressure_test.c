#include "pressure_test.h"
#include "pmp_user.h"
#include "os.h"

unsigned long read_cycles(void)
{
    unsigned long cycles;
    asm volatile ("rdcycle %0" : "=r" (cycles));
    return cycles;
}

void muti_test(){
    testNoOverlap10();
}

void testNoOverlap10(){
    unsigned int interval = 4;
    unsigned int start_bound = 0x87E00000;
    unsigned int start = start_bound;
    unsigned int end_bound = 0x87F00000;
    unsigned int end = start;

    unsigned long avg_time_request = 0;
    unsigned long avg_time_access = 0;
    unsigned long avg_time_exception = 0;
    unsigned long avg_time_free = 0;
    int local_exception_count = 0;
    
    for(int i = 0; i < 10; i++){
        unsigned long cycles_start = read_cycles();
        pmp_isolation_request(addr2pmpaddr( start ), addr2pmpaddr( start + interval ), 0x0e, i); 
        unsigned long cycles_end = read_cycles();
        avg_time_request += cycles_end - cycles_start;
        start += interval;
        end = start;
    }
    avg_time_request /= 10;
    start = start_bound;
    
    for(int i = 0; i < 10; i++){
        unsigned long cycles_start = read_cycles();
        asm volatile ("lb t0, 0(%0)" :: "r" (start));
        unsigned long cycles_end = read_cycles();
        avg_time_access += cycles_end - cycles_start;
        if(getNset_exception_count() == 1){
            local_exception_count += 1;
            avg_time_exception += cycles_end - cycles_start;
        }
        start += interval;
    }
    avg_time_access /= 10;
    avg_time_exception /= local_exception_count;
    start = start_bound;

    for(int i = 0; i < 10; i++){
        unsigned long cycles_start = read_cycles();
        pmp_free(i);
        unsigned long cycles_end = read_cycles();
        avg_time_free += cycles_end - cycles_start;
    }
    avg_time_free /= 10;

    // print the result
    printf("average request time = %lu", avg_time_request);
    printf("average access time = %lu", avg_time_access);
    printf("average exception time = %lu", avg_time_exception);
    printf("average free time = %lu", avg_time_free);
}

void testNoOverlap100(){
}

void testNoOverlap1000(){
}

void testNoOverlap10000(){
}

void testOverlap10(){
}

void testOverlap100(){
}

void testOverlap1000(){
}

void testOverlap10000(){
}

void testHighOverlap(){
}

void testFree100(){
}

void testFree1000(){
}
