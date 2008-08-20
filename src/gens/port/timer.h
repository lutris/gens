#ifndef __TIMER_H__
#define __TIMER_H__

void init_timer();
void GetPerformanceFrequency(long long *freq);
void GetPerformanceCounter(long long *now);
unsigned long gettime();

#endif
