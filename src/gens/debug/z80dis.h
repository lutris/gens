#ifndef _Z80DIS_H_
#define _Z80DIS_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_DEBUG

int z80dis(unsigned char *buf, int *Counter, char str[128]);

#endif /* GENS_DEBUG */

#endif /* _Z80DIS_H_ */
