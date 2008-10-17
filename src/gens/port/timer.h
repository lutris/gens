#ifndef GENS_TIMER_H
#define GENS_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void init_timer(void);
unsigned int gettime(void);

void QueryPerformanceFrequency(long long *freq);
void QueryPerformanceCounter(long long *now);

#ifdef __cplusplus
}
#endif

#endif /* GENS_TIMER_H */
