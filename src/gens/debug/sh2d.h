#ifndef GENS_SH2D_H
#define GENS_SH2D_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef GENS_DEBUG

void SH2Disasm(char *c, unsigned v_addr, unsigned short op_norm, int mode);

#endif /* GENS_DEBUG */

#ifdef __cplusplus
}
#endif

#endif /* GENS_SH2D_H */
