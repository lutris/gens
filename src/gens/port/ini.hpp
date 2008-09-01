/**
 * Gens: INI file handler.
 */

#ifndef GENS_INI_HPP
#define GENS_INI_HPP

#ifdef __cplusplus
extern "C" {
#endif

void INI_LoadConfig(const char* filename);
void INI_Clear(void);
void INI_SaveConfig(const char* filename);

int INI_GetInt(const char* section, const char* key, const int def);
void INI_GetString(const char* section, const char* key, const char* def, char* buf, const int size);
void INI_WriteInt(const char* section, const char* key, const int value);
void INI_WriteString(const char* section, const char* key, const char* value);
void INI_SaveConfig(const char* filename);

int GetPrivateProfileInt(const char *section, const char *var, const int def, const char *filename);
void GetPrivateProfileString(const char *section, const char *var, const char *def, char *get, const int length, const char *filename);
void WritePrivateProfileString(const char *section, const char *var, const char *var_name, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* GENS_INI_HPP */
