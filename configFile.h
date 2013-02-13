#ifndef _CONFIG_FILE_H_
#define _CONFIG_FILE_H_

#include <BaroboConfigFile.h>

class ConfigFile
{
  public:
    ConfigFile();
    ~ConfigFile();
    int read(const char* path);
    int numEntries();
    const char* getEntry(int index);
    bool entryExists(const char* entry);
    int addEntry(const char* entry);
    int insertEntry(const char* entry, int index);
    int moveEntryDown(int index);
    int moveEntryUp(int index);
    int remove(int index);
    int rename(const char* newName, int index);
    int addDongle(const char* entry);
    const char* getDongle(int index);
    int removeDongle(const char *name);
    int write();

  private:
    bcf_t* _bcf;
};


#endif


