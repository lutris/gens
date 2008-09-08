/**
 * Gens: INI file handler.
 */

#ifndef GENS_INI_HPP
#define GENS_INI_HPP

#ifdef __cplusplus

#include <string.h>

#include <string>
#include <map>
using std::string;
using std::map;
using std::pair;

// Case-insensitive search from http://www.experts-exchange.com/Programming/Languages/CPP/Q_23025401.html
class __lesscasecmp
{
	public:
		bool operator() (const string& a, const string& b) const
		{
			return (strcasecmp(a.c_str(), b.c_str()) < 0);
		}
};

typedef map<string, string, __lesscasecmp> iniSection;
typedef map<string, iniSection, __lesscasecmp> iniFile;

class INI
{
	public:
		INI();
		INI(const string& filename);
		~INI();
		
		// Load an INI file.
		void load(const string& filename);
		
		// Save the INI file.
		void save(const string& filename);
		
		// Clear the loaded INI file.
		void clear(void);
		
		// Get settings from the loaded INI file.
		int getInt(const string& section, const string& key, const int def);
		bool getBool(const string& section, const string& key, const bool def);
		string getString(const string& section, const string& key, const string& def);
		void getString(const string& section, const string& key, const string& def, char *buf, int size);
		
		// Write settings to the loaded INI file.
		void writeInt(const string& section, const string& key, const int value);
		void writeBool(const string& section, const string& key, const bool value);
		void writeString(const string& section, const string& key, const string& value);
	
	protected:
		iniFile m_INI;
};

#endif

#ifdef __cplusplus
extern "C" {
#endif

void INI_LoadConfig(const char* filename);
void INI_Clear(void);
void INI_SaveConfig(const char* filename);

int INI_GetInt(const char* section, const char* key, const int def);
bool INI_GetBool(const char* section, const char* key, const bool def);
void INI_GetString(const char* section, const char* key, const char* def, char* buf, const int size);
void INI_WriteInt(const char* section, const char* key, const int value);
void INI_WriteBool(const char* section, const char* key, const bool value);
void INI_WriteString(const char* section, const char* key, const char* value);
void INI_SaveConfig(const char* filename);

int GetPrivateProfileInt(const char *section, const char *var, const int def, const char *filename);
void GetPrivateProfileString(const char *section, const char *var, const char *def, char *get, const int length, const char *filename);
void WritePrivateProfileString(const char *section, const char *var, const char *var_name, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* GENS_INI_HPP */
