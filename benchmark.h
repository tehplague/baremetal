/*
 * =====================================================================================
 *
 *       Filename:  benchmark.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  16.11.2011 14:19:40
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Georg Wassen (gw) (), 
 *        Company:  
 *
 * =====================================================================================
 */

#ifndef BENCHMARK_H
#define BENCHMARK_H



void hourglass(unsigned sec);
void load_until_flag(void *buffer, size_t size, size_t stride, flag_t *flag);
uint64_t range_stride(void *buffer, size_t range, size_t stride);

typedef enum {AT_READ, AT_WRITE, AT_UPDATE, AT_ATOMIC} access_t;
void worker(volatile unsigned long *p_buffer, size_t range, size_t stride, access_t type, unsigned sec);


void bench_hourglass(barrier_t *barr);
void bench_hourglass_worker(barrier_t *barr, void *p_contender);
void bench_hourglass_hyperthread(barrier_t *barr);
void bench_worker(barrier_t *barr, void *p_buffer, void *p_contender);
void bench_worker_cut(barrier_t *barr, void *p_buffer, void *p_contender, size_t worker_size);
void bench_mem(barrier_t *barr, void *p_buffer, void *p_contender);

#endif  // BENCHMARK_H
