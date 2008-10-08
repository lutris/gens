/**
 * Gens: INI file handler.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ini.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
using std::endl;
using std::ios;
using std::ifstream;
using std::ofstream;
using std::stringstream;


INI::INI()
{
	// Standard constructor. Nothing's needed
}


INI::INI(const string& filename)
{
	// Load an INI file.
	load(filename);
}


INI::~INI()
{
}


/**
 * load(): Load the specified INI file into m_INI.
 * @param filename Filename of the INI file to load.
 */
void INI::load(const string& filename)
{
	// Load the specified INI file.
	string tmp;
	
	// Clear the map.
	m_INI.clear();
	
	// Open the file.
	ifstream cfgFile(filename.c_str());
	
	iniFile::iterator iniSectIter;
	iniSection curSection; string curSectionName;
	iniSection::iterator sectKeyIter;
	
	unsigned int commentPos;
	unsigned int equalsPos;
	string curKey, curValue;
	
	if (!cfgFile.is_open())
	{
		// Error opening the INI file.
		fprintf(stderr, "%s(): Error opening INI file %s\n", __func__, filename.c_str());
		return;
	}
	
	
	while (!cfgFile.eof())
	{
		getline(cfgFile, tmp);
		
		// Parse the line.
		
		// Remove any comments from the line.
		commentPos = tmp.find(";");
		if (commentPos != string::npos)
		{
			// Comment found.
			tmp = tmp.substr(0, commentPos - 1);
		}
		
		if (tmp.length() == 0)
		{
			// Empty string.
			continue;
		}
		
		if (tmp.substr(0, 1) == "[" &&
			tmp.substr(tmp.length() - 1, 1) == "]")
		{
			// Section header.
			
			// If there's a section right now, add it to the map.
			if (curSectionName.length() > 0)
			{
				m_INI.insert(pair<string, iniSection>(curSectionName, curSection));
				curSection.clear();
				curSectionName.clear();
			}
			
			// Set the section.
			curSectionName = tmp.substr(1, tmp.length() - 2);
			
			// Check if this section already exists.
			iniSectIter = m_INI.find(curSectionName);
			if (iniSectIter != m_INI.end())
			{
				// Section already exists.
				// Copy it and delete the one in the map.
				curSectionName = (*sectKeyIter).first;
				curSection = (*iniSectIter).second;
				m_INI.erase(iniSectIter);
			}
		}
		else if (curSectionName.length() > 0)
		{
			// Check if there's an "=", indicating a key value.
			equalsPos = tmp.find("=");
			if (equalsPos == string::npos)
			{
				// No key value.
				continue;
			}
			
			// Key value.
			curKey = tmp.substr(0, equalsPos);
			curValue = tmp.substr(equalsPos + 1);
			
			// Check if this key already exists in the current section.
			sectKeyIter = curSection.find(curKey);
			if (sectKeyIter != curSection.end())
			{
				// Already exists. Delete the existing one.
				curSection.erase(sectKeyIter);
			}
			
			// Add the key.
			curSection.insert(pair<string, string>(curKey, curValue));
		}
	}
	
	// If a section is still being added, add it to the config.
	if (curSectionName.length() > 0)
	{
		m_INI.insert(pair<string, iniSection>(curSectionName, curSection));
		curSection.clear();
		curSectionName.clear();
	}
	
	cfgFile.close();
}


/**
 * INI_Clear(): Clear the loaded INI file.
 */
void INI::clear(void)
{
	m_INI.clear();
}


/**
 * getInt(): Get an integer value from the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param def Default value if the key isn't found.
 * @return Integer value.
 */
int INI::getInt(const string& section, const string& key, const int def)
{
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	iniSectIter = m_INI.find(section);
	if (iniSectIter == m_INI.end())
	{
		// Section not found.
		return def;
	}
	
	sectKeyIter = (*iniSectIter).second.find(key);
	if (sectKeyIter == (*iniSectIter).second.end())
	{
		// Key not found.
		return def;
	}
	
	// Found the key.
	return atoi((*sectKeyIter).second.c_str());
}


/**
 * getBool(): Get a boolean value from the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param def Default value if the key isn't found.
 * @return Boolean value.
 */
bool INI::getBool(const string& section, const string& key, const bool def)
{
	return getInt(section, key, def);
}


/**
 * getString(): Get a string value from the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param def Default value if the key isn't found.
 * @return String value.
 */
string INI::getString(const string& section, const string& key, const string& def)
{
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	iniSectIter = m_INI.find(section);
	if (iniSectIter == m_INI.end())
	{
		// Section not found.
		return def;
	}
	
	sectKeyIter = (*iniSectIter).second.find(key);
	if (sectKeyIter == (*iniSectIter).second.end())
	{
		// Key not found.
		return def;
	}
	
	// Found the key.
	return (*sectKeyIter).second;
}


/**
 * getString(): Get a string value from the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param def Default value if the key isn't found.
 * @param buf Buffer to store the string value in.
 * @param size Size of the buffer.
 */
void INI::getString(const string& section, const string& key, const string& def,
		    char *buf, unsigned int size)
{
	string tmp;
	tmp = getString(section, key, def);
	strncpy(buf, tmp.c_str(), size);
	
	if (tmp.length() >= size)
	{
		// String size exceeds the buffer. Set the last byte in the buffer to NULL.
		buf[size - 1] = 0x00;
	}
}


/**
 * writeInt(): Writes an integer value to the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param value Integer value to write.
 */
void INI::writeInt(const string& section, const string& key, const int value)
{
	stringstream out;
	out << value;
	writeString(section, key, out.str().c_str());
}


/**
 * writeBool(): Writes a boolean value to the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param value Integer value to write.
 */
void INI::writeBool(const string& section, const string& key, const bool value)
{
	writeString(section, key, (value ? "1" : "0"));
}


/**
 * INI_WriteString(): Writes a string value to the loaded INI file.
 * @param section Section to get from.
 * @param key Key to get from.
 * @param value String value to write.
 */
void INI::writeString(const string& section, const string& key, const string& value)
{
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	iniSectIter = m_INI.find(section);
	if (iniSectIter == m_INI.end())
	{
		// Section not found. Create it.
		iniSection newSection;
		m_INI.insert(pair<string, iniSection>(section, newSection));
		iniSectIter = m_INI.find(section);
		if (iniSectIter == m_INI.end())
		{
			// Error creating the new section.
			fprintf(stderr, "%s(): Error creating new INI section: %s\n", __func__, section.c_str());
			return;
		}
	}
	
	// Check if this key already exists.
	sectKeyIter = (*iniSectIter).second.find(key);
	if (sectKeyIter != (*iniSectIter).second.end())
	{
		// Key found. Delete it.
		(*iniSectIter).second.erase(sectKeyIter);
	}
	
	// Create the key.
	(*iniSectIter).second.insert(pair<string, string>(key, value));
	
	// Found the key.
	return;
}


/**
 * save(): Save m_INI to the specified INI file.
 * @param filename Filename of the INI file to save to.
 */
void INI::save(const string& filename)
{
	// Load the specified INI file.
	string tmp;
	
	// Open the file.
	ofstream cfgFile(filename.c_str());
	
	iniFile::iterator iniSectIter;
	iniSection::iterator sectKeyIter;
	
	if (!cfgFile.is_open())
	{
		// Error opening the INI file.
		fprintf(stderr, "%s(): Error opening INI file %s", __func__, filename.c_str());
		return;
	}
	
	// Write to the INI file.
	for (iniSectIter = m_INI.begin(); iniSectIter != m_INI.end(); iniSectIter++)
	{
		// Write the section header.
		cfgFile << "[" << (*iniSectIter).first << "]" << endl;
		
		// Write all keys.
		for (sectKeyIter = (*iniSectIter).second.begin();
		     sectKeyIter != (*iniSectIter).second.end();
		     sectKeyIter++)
		{
			cfgFile << (*sectKeyIter).first << "=" << (*sectKeyIter).second << endl;
		}
		
		// Add a newline between sections.
		cfgFile << endl;
	}
	
	// Done.
	
	cfgFile.close();
}


/* Old INI code below here. */

#ifndef __WIN32__

static void WriteKey(const char *var, const char *var_name, FILE * file)
{
	fwrite (var, strlen (var), 1, file);
	fwrite ("=", 1, 1, file);
	fwrite (var_name, strlen (var_name), 1, file);
	fwrite ("\n", 1, 1, file);
}

static void InsertSectionKey(const char *section, const char *var,
			     const char *var_name, FILE * file, int saut)
{
	if (saut)
		fwrite("\n", 1, 1, file);
	fwrite("[", 1, 1, file);
	fwrite(section, strlen(section), 1, file);
	fwrite("]\n", 2, 1, file);
	WriteKey(var, var_name, file);
}

static char* SearchKey(const char *fromWhere, const char *key)
{
	char *found = NULL;
	char *ext_key;
	
	ext_key = (char*)malloc(1 + strlen (key) + 1 + 1);	// \n key \0
	ext_key[0] = '\n';
	ext_key[1] = '\0';
	strcat (ext_key, key);
	found = strstr (fromWhere, ext_key);
	if (found)
		return found + 1;
	else
		return NULL;
}

static char *SearchSection(const char *buf, const char *section)
{
	char *found = NULL;
	char *ext_section;
	
	ext_section = (char*)malloc(1 + strlen (section) + 1 + 1);	// [ section ] \0 
	ext_section[0] = '[';
	ext_section[1] = '\0';
	strcat (ext_section, section);
	strcat (ext_section, "]");
	found = strstr (buf, ext_section);
	if (found)
		return found + 1;
	else
		return NULL;
}


static char* file2buf (const char *filename)
{
	char *buf;
	int filesize;
	struct stat sb;
	FILE *file;
	int res;
	
	res = stat (filename, &sb);
	if (-1 == res)
		return NULL;
	filesize = sb.st_size - 1;
	if (filesize >= 0)
	{
		buf = (char *) calloc (filesize + 1, 1);
		file = fopen (filename, "r");
		fread (buf, filesize, 1, file);
		fclose (file);
		buf[filesize] = '\0';
	}
	else
	{
		buf = (char*)calloc(1, 1);
	}
	return buf;
}


int GetPrivateProfileInt(const char *section, const char *var,
			 int def, const char *filename)
{
	char get[10];
	char defstr[10];
	int res;
	
	memset(get, 0, 10);
	memset(defstr, 0, 10);
	
	if (!filename)
		return def;
	
	sprintf (defstr, "%d", def);
	
	GetPrivateProfileString (section, var, defstr, get, 10, filename);
	sscanf(get, "%d", &res);
	
	return res;
}

void GetPrivateProfileString(const char *section, const char *var,
			     const char *def, char *get, int length,
			     const char *filename)
{
	char *buf;
	char *section_found;
	char *key_found;
	
	memset (get, 0, length);
	
	if (!filename)
	{
		strncpy(get, def, length);
		return;
	}
	
	buf = file2buf(filename);
	if (buf)
	{
		section_found = SearchSection(buf, section);
		if (section_found)
		{
			key_found = SearchKey(section_found, var);
			if (key_found)
			{
				char *val_start;
				char *val_end;
				val_start = strchr (key_found, '=') + 1;
				val_end = val_start;
				while ((*val_end != '\0') && (*val_end != '\n') && (*val_end != '\r'))
					val_end++;
				strncpy (get, val_start, val_end - val_start);
				get[val_end - val_start] = '\0';
				return;
			}
		}
	}
	strncpy (get, def, length);
}


void WritePrivateProfileString(const char *section, const char *var,
			       const char *var_name, const char *filename)
{
	FILE *file;
	struct stat sb;
	int res;
	
	res = stat(filename, &sb);
	if (-1 == res)		// fichier n'existe pas
	{
		file = fopen (filename, "w");
		InsertSectionKey(section, var, var_name, file, 0);
		fclose(file);
	}
	else
	{
		int filesize;
		filesize = sb.st_size - 1;	// on ne lit pas EOT
		if (filesize >= 0)
		{
			char *buf;
			char *section_begin;
			int size1, size2;
			
			buf = file2buf(filename);
			
			section_begin = SearchSection(buf, section);
			if (NULL != section_begin)	//section existe déjà
			{
				char *key_begin;
				key_begin = SearchKey(section_begin, var);
				if (NULL != key_begin)	// la clé existe : on copie la partie d'avant l'ancienne clé, la nouvelle clé, puis la partie d'après l'ancienne clé
				{
					int old_key_size;
					char *key_end;
					
					size1 = key_begin - buf;
					key_end = strstr(key_begin, "\n");
					if (NULL != key_end)
					{
						old_key_size = key_end - key_begin + 1;
						size2 = filesize - (size1 + old_key_size);
						file = fopen(filename, "w");
						fwrite(buf, size1, 1, file);
						WriteKey(var, var_name, file);
						fwrite(buf + size1 + old_key_size, size2, 1, file);
						fclose(file);
					}
					else
					{		// la clé est la dernière du fichier
						file = fopen(filename, "w");
						fwrite(buf, size1, 1, file);
						WriteKey(var, var_name, file);
						fclose(file);
					}

				}
				else
				{		// clé n'existe pas : on la rajoute avant la prochaine section
					char *next_section_begin;
					next_section_begin = strstr (section_begin, "*");
					if (NULL == next_section_begin)	// la section recherchée est unique : il suffit de copier la clé à la fin du fichier
					{
						file = fopen (filename, "a");
						WriteKey(var, var_name, file);
						fclose(file);
					}
					else
					{		// on insère la clé avant la prochaine section
						size1 = next_section_begin - buf;
						size2 = filesize - size1;
						file = fopen(filename, "w");
						fwrite(buf, size1, 1, file);
						WriteKey(var, var_name, file);
						fwrite(buf + size1, size2, 1, file);
						fclose(file);
					}
				}
			}
			else
			{			//section n'existe pas : on la rajoute ainsi que la clé
				file = fopen(filename, "a");
				InsertSectionKey(section, var, var_name, file, 1);
				fclose(file);
			}
			free (buf);
		}
		else
		{			//taille fichier < 0
			file = fopen(filename, "w");
			InsertSectionKey(section, var, var_name, file, 0);
			fclose(file);
		}
	}
}

#endif /* __WIN32__ */
