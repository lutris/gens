#include <string.h>
#include <unistd.h>

#include "port.h"
#include "timer.h"

unsigned int GetTickCount(void)
{
	return gettime();
}
