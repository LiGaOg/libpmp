#include "pressure_test.h"
#include "pmp_system_library.h"

unsigned long read_cycles(void)
{
    unsigned long cycles;
    asm volatile ("rdcycle %0" : "=r" (cycles));
    return cycles;
}

void muti_test(){
    testNoOverlap(10);
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
    unsigned int interval = (int)( space_len * ( 1 - overlap_ratio ) );

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

void testHighOverlap(unsigned int request_num, unsigned int block_interval){
    // This function will generate high overlap 
    // block interval = paramter * 4, smaller is more complicated
    // request_num = request block number
    unsigned int interval = block_interval * 4;
    unsigned int start_bound = 0x87E00000;
    unsigned int end_bound = 0x87F00000;
    unsigned int centerPoint = start_bound + 100;

    unsigned long avg_time_request = 0;
    unsigned long avg_time_access = 0;
    unsigned long avg_time_exception = 0;
    unsigned long avg_time_free = 0;
    int local_exception_count = 0;

    unsigned int pmp_id_count = 0;

    int privilege[8] = {0,1,2,4,3,5,6,7};

    for(int i = 0; i < request_num; i++){
        // left
        unsigned int leftPoint = centerPoint - 40;
        for(int j = 1; j <= 100; j++){
            unsigned int leftbound = leftPoint - 4 * j;
            unsigned int rightbound = leftPoint + 4 * j;
            if(leftbound < start_bound)leftbound = start_bound;
            if(rightbound > end_bound)rightbound = end_bound;
            unsigned int seed = ((((pmp_id_count * 214013U + 2531011U) & 0xffffffff) >> 16) & 0x7fff) % 8;
            unsigned long cycles_start = read_cycles();
            pmp_isolation_request(addr2pmpaddr( leftbound ), addr2pmpaddr( rightbound ), privilege[seed], pmp_id_count);
            unsigned long cycles_end = read_cycles();
            pmp_id_count += 1;
            avg_time_request += cycles_end - cycles_start;
        }
        // center 
         for(int j = 1; j <= 100; j++){
            unsigned int leftbound = centerPoint - 4 * j;
            unsigned int rightbound = centerPoint + 4 * j;
            if(leftbound < start_bound)leftbound = start_bound;
            if(rightbound > end_bound)rightbound = end_bound;
            unsigned int seed = ((((pmp_id_count * 214013U + 2531011U) & 0xffffffff) >> 16) & 0x7fff) % 8;
            unsigned long cycles_start = read_cycles();
            pmp_isolation_request(addr2pmpaddr( leftbound ), addr2pmpaddr( rightbound ), privilege[seed], pmp_id_count);
            unsigned long cycles_end = read_cycles();
            pmp_id_count += 1;
            avg_time_request += cycles_end - cycles_start;
        }
        // right 
        unsigned int rightPoint = centerPoint + 40;
        for(int j = 1; j <= 100; j++){
            unsigned int leftbound = rightPoint - 4 * j;
            unsigned int rightbound = rightPoint + 4 * j;
            if(leftbound < start_bound)leftbound = start_bound;
            if(rightbound > end_bound)rightbound = end_bound;
            unsigned int seed = ((((pmp_id_count * 214013U + 2531011U) & 0xffffffff) >> 16) & 0x7fff) % 8;
            unsigned long cycles_start = read_cycles();
            pmp_isolation_request(addr2pmpaddr( leftbound ), addr2pmpaddr( rightbound ), privilege[seed], pmp_id_count);
            unsigned long cycles_end = read_cycles();
            pmp_id_count += 1;
            avg_time_request += cycles_end - cycles_start;
        }
        centerPoint += interval;
    }
    // request end
    avg_time_request /= pmp_id_count;
    
    // To simpify, just use the center
    for(int i = 1; i <= request_num; i++){
        unsigned long cycles_start = read_cycles();
        asm volatile ("lb t0, 0(%0)" :: "r" (start_bound + 100 * i));
        unsigned long cycles_end = read_cycles();
        avg_time_access += cycles_end - cycles_start;
        if(getNset_exception_count() == 1){
            local_exception_count += 1;
            avg_time_exception += cycles_end - cycles_start;
        }
    }
    avg_time_access /= request_num;
    avg_time_exception /= local_exception_count;


    for(int i = 0; i < pmp_id_count; i++){
        unsigned long cycles_start = read_cycles();
        pmp_free(i);
        unsigned long cycles_end = read_cycles();
        avg_time_free += cycles_end - cycles_start;
    }
    avg_time_free /= pmp_id_count;

    // print the result
    printf("average request time = %lu", avg_time_request);
    printf("average access time = %lu", avg_time_access);
    printf("average exception time = %lu", avg_time_exception);
    printf("average free time = %lu", avg_time_free);
}
