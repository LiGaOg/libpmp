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

void testNoOverlap(unsigned int request_num){
    /**
        request_num should be 4n (n=1,2,3,4,...)
    */
    unsigned int start_bound = 0x87E00000;
    unsigned int start = start_bound;
    unsigned int end_bound = 0x87F00000;
    unsigned int end = start;
    unsigned int interval = (end_bound - start_bound) / request_num;

    unsigned long avg_time_request = 0;
    unsigned long avg_time_access = 0;
    unsigned long avg_time_exception = 0;
    unsigned long avg_time_free = 0;
    int local_exception_count = 0;
    
    for(int i = 0; i < request_num; i++){
        unsigned long cycles_start = read_cycles();
        pmp_isolation_request(addr2pmpaddr( start ), addr2pmpaddr( start + interval ), 0x0e, i); 
        unsigned long cycles_end = read_cycles();
        avg_time_request += cycles_end - cycles_start;
        start += interval;
        end = start;
    }
    avg_time_request /= request_num;
    start = start_bound;
    
    for(int i = 0; i < request_num; i++){
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
    avg_time_access /= request_num;
    avg_time_exception /= local_exception_count;
    start = start_bound;

    for(int i = 0; i < request_num; i++){
        unsigned long cycles_start = read_cycles();
        pmp_free(i);
        unsigned long cycles_end = read_cycles();
        avg_time_free += cycles_end - cycles_start;
    }
    avg_time_free /= request_num;

    // print the result
    printf("average request time = %lu", avg_time_request);
    printf("average access time = %lu", avg_time_access);
    printf("average exception time = %lu", avg_time_exception);
    printf("average free time = %lu", avg_time_free);
}

void testSequentialOverlap(unsigned int request_num, unsigned int space_len, float overlap_ratio){
    /**
        Allocate memory forward one by one

        request_num: request_num * space_len <= 0x100000, cannot be NULL
        space_len: 4n (n=1,2,3,4,...) if it isn't NULL, it can be NULL
        overlap_ratio: space_len * overlap_ratio should be 4n (n=1,2,3,4,...), cannot be NULL
    */
    unsigned int start_bound = 0x87E00000;
    unsigned int start = start_bound;
    unsigned int end_bound = 0x87F00000;
    unsigned int end = start;
    int privilege[8] = {0,1,2,4,3,5,6,7};
    if ( space_len == NULL){
        space_len = (end_bound - start_bound) / request_num;
    }
    unsigned int interval = int( space_len * ( 1 - overlap_ratio ) );

    unsigned long avg_time_request = 0;
    unsigned long avg_time_access = 0;
    unsigned long avg_time_exception = 0;
    unsigned long avg_time_free = 0;
    int local_exception_count = 0;

    for(int i = 0; i < request_num; i++){
        unsigned long cycles_start = read_cycles();
        pmp_isolation_request(addr2pmpaddr( start ), addr2pmpaddr( start + space_len ), privilege[i%8], i);
        unsigned long cycles_end = read_cycles();
        avg_time_request += cycles_end - cycles_start;
        start += interval;
        end = start;
    }
    avg_time_request /= request_num;
    start = start_bound;

    for(int i = 0; i < request_num; i++){
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
    avg_time_access /= request_num;
    avg_time_exception /= local_exception_count;
    start = start_bound;

    for(int i = 0; i < request_num; i++){
        unsigned long cycles_start = read_cycles();
        pmp_free(i);
        unsigned long cycles_end = read_cycles();
        avg_time_free += cycles_end - cycles_start;
    }
    avg_time_free /= request_num;

    // print the result
    printf("average request time = %lu", avg_time_request);
    printf("average access time = %lu", avg_time_access);
    printf("average exception time = %lu", avg_time_exception);
    printf("average free time = %lu", avg_time_free);
}

void testCentricOverlap(unsigned int request_num, unsigned int init_space_len, unsigned int extend_radius){
    /**
        Allocate memory around a default pivot at 0x87E80000

        request_num: request_num * space_len <= 0x100000, cannot be NULL
        init_space_len: 8n (n=1,2,3,4,...), cannot be NULL
        extend_radius: 4n (n=1,2,3,4,...), the space of next allocated memory is 8n larger than the previous one's
    */
    unsigned int pivot = 0x87E80000;
    unsigned int start = pivot - init_space_len / 2;
    unsigned int space_len = init_space_len;
    int privilege[8] = {0,1,2,4,3,5,6,7};

    unsigned long avg_time_request = 0;
    unsigned long avg_time_access = 0;
    unsigned long avg_time_exception = 0;
    unsigned long avg_time_free = 0;
    int local_exception_count = 0;

    for(int i = 0; i < request_num; i++){
        unsigned long cycles_start = read_cycles();
        pmp_isolation_request(addr2pmpaddr( start ), addr2pmpaddr( start + space_len ), privilege[i%8], i);
        unsigned long cycles_end = read_cycles();
        avg_time_request += cycles_end - cycles_start;
        start -= extend_radius;
        space_len += 2 * extend_radius;
    }
    avg_time_request /= request_num;
    start = pivot - init_space_len / 2;
    space_len = init_space_len;

    for(int i = 0; i < request_num; i++){
        unsigned long cycles_start = read_cycles();
        asm volatile ("lb t0, 0(%0)" :: "r" (start));
        unsigned long cycles_end = read_cycles();
        avg_time_access += cycles_end - cycles_start;
        if(getNset_exception_count() == 1){
            local_exception_count += 1;
            avg_time_exception += cycles_end - cycles_start;
        }
        start -= extend_radius;
    }
    avg_time_access /= request_num;
    avg_time_exception /= local_exception_count;
    start = pivot - init_space_len / 2;

    for(int i = 0; i < request_num; i++){
        unsigned long cycles_start = read_cycles();
        pmp_free(i);
        unsigned long cycles_end = read_cycles();
        avg_time_free += cycles_end - cycles_start;
    }
    avg_time_free /= request_num;

    // print the result
    printf("average request time = %lu", avg_time_request);
    printf("average access time = %lu", avg_time_access);
    printf("average exception time = %lu", avg_time_exception);
    printf("average free time = %lu", avg_time_free);
}

void testHighOverlap(){
}

void testFree100(){
}

void testFree1000(){
}
