/**
 * Gens: INI file handler. (Old Win32 replacement code)
 */

#ifndef GENS_INI_OLD_H
#define GENS_INI_OLD_H

#ifdef __cplusplus
extern "C" {
#endif

int GetPrivateProfileInt(const char *section, const char *var,
			 const int def, const char *filename);

void GetPrivateProfileString(const char *section, const char *var,
			     const char *def, char *get, const int length,
			     const char *filename);

void WritePrivateProfileString(const char *section, const char *var,
			       const char *var_name, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* GENS_INI_OLD_H */
