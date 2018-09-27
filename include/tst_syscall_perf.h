#ifndef TST_SYSCALL_PERF_H
#define TST_SYSCALL_PERF_H
#include <inttypes.h>
#include <stdint.h>

#define SYSCALL_PERF_GET_TICKS        syscall_perf_get_ticks
#define SYSCALL_PERF_MEASURE          syscall_perf_measure
#define SYSCALL_PERF_MEASURE_AVERAGE  syscall_perf_measure_bucket

#define SYSCALL_PERF_SET_CPU usc_set_cpu
void usc_set_cpu();

#define SYSCALL_PERF_ALLOCATE_BUCKET usc_allocate_perf_bucket 
void usc_allocate_perf_bucket(uint64_t bucket_size);

#define SYSCALL_PERF_ADD_SAMPLE usc_add_to_perf_bucket
void usc_add_to_perf_bucket(uint64_t cycle);

#define SYSCALL_GET_LOOP_COUNTER usc_get_lc
uint64_t usc_get_lc();

uint64_t* usc_get_perf_bucket();

static __inline__ uint64_t rdtsc(void)
{
	uint32_t hi, lo;
	__asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
	return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}

static inline uint64_t syscall_perf_get_ticks()
{
    return rdtsc();
}

static inline void syscall_perf_measure(uint64_t start_tick, uint64_t end_tick)
{
    uint64_t cpu_cycles = end_tick - start_tick;
    printf(" Number of CPU cycles: %"PRIu64" \n", cpu_cycles);
}

static inline void syscall_perf_measure_bucket()
{
    uint64_t sum = 0, index = 0, total_samples = usc_get_lc();
    uint64_t *bucket = usc_get_perf_bucket();

    while (index < total_samples)
    {
        sum += bucket[index++];
    }
    printf(" Average CPU cycle: %"PRIu64" \n", sum / total_samples);
}
#endif
