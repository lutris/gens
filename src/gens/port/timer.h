#ifndef GENS_TIMER_H
#define GENS_TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

void init_timer(void);
//unsigned int gettime(void);

unsigned int GetTickCount(void);
void QueryPerformanceFrequency(long long *frequency);
void QueryPerformanceCounter(long long *counter);

#ifdef __cplusplus
}
#endif

#endif /* GENS_TIMER_H */
