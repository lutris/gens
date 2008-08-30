#ifndef GENS_M68KD_H
#define GENS_M68KD_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_DEBUG

char *M68KDisasm(unsigned short (*NW)(), unsigned int (*NL)());

#endif /* GENS_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* GENS_M68KD_H */
