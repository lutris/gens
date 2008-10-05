#include <unistd.h>
#if HAVE_LIBRT
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <stddef.h>
#include "timer.h"


#define UCLOCKS_PER_SEC		1000000
static long long cpu_speed = 0;


// return time in micro-second
static inline long long system_time(void)
{
#if HAVE_LIBRT
	struct timespec tv;
	clock_gettime(CLOCK_REALTIME, &tv);
	return (tv.tv_sec * 1000000) + (tv.tv_nsec / 1000);
#else
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec * 1000000) + (tv.tv_usec);
#endif
}


/**
 * read_rdtsc(): Read the time stamp counter.
 * @return Time stamp counter.
 */
static inline long long read_rdtsc(void)
{
	long long result;
	__asm__ __volatile__ ("rdtsc":"=A" (result));
	return result;
}


// based on mame
static inline long long get_cpu_speed(void)
{
	long long a, b, start, end;
	
	a = system_time();
	do
	{
		b = system_time();
	}
	while (a == b);
	
	start = read_rdtsc();
	
	do
	{
		a = system_time();
	}
	while (a - b < UCLOCKS_PER_SEC / 4);
	
	end = read_rdtsc();
	
	return (end - start) * 4;
}


unsigned int gettime(void)
{
	return system_time() / 1000;
}


void init_timer(void)
{
	cpu_speed = get_cpu_speed();
}


void GetPerformanceFrequency(long long *freq)
{
	*freq = cpu_speed;
}

void GetPerformanceCounter(long long *now)
{
	*now = read_rdtsc();
}
