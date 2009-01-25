/**
 * Gens: Genesis (Mega Drive) initialization and main loop code.
 */

#ifndef GENS_MD_HPP
#define GENS_MD_HPP

#include "util/file/rom.hpp"

#ifdef __cplusplus
extern "C" {
#endif

void Detect_Country_Genesis(void);

void Init_Genesis_Bios(void);
void Init_Genesis_SRAM(ROM_t* MD_ROM);
int Init_Genesis(ROM_t* MD_ROM);
void Reset_Genesis(void);
int Do_VDP_Only(void);

#ifdef __cplusplus
}
#endif

#endif /* GENS_MD_HPP */
