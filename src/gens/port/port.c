#include <string.h>
#include <unistd.h>

#include "port.h"
#include "timer.h"

unsigned int GetTickCount(void)
{
	return gettime();
}

void SetCurrentDirectory(const char *directory)
{
	chdir(directory);
}

int GetCurrentDirectory(int size, char *buf)
{
	getcwd(buf, size);
	return strlen(buf);
}
