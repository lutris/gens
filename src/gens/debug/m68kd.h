#ifndef _M68KD_H_
#define _M68KD_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_DEBUG

char *M68KDisasm(unsigned short (*NW)(), unsigned int (*NL)());

#endif /* GENS_DEBUG */

#endif /* _M68KD_H_ */
