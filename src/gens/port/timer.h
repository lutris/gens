#ifndef GENS_TIMER_H
#define GENS_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void init_timer(void);
void GetPerformanceFrequency(long long *freq);
void GetPerformanceCounter(long long *now);
unsigned int gettime(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_TIMER_H */
