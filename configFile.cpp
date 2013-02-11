#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <stdlib.h>

#include "configFile.h"

using namespace std;

ConfigFile::ConfigFile()
{
  _bcf = BCF_New();
}

ConfigFile::~ConfigFile()
{
  BCF_Destroy(_bcf);
}

int ConfigFile::read(const char* path)
{
  return BCF_Read(_bcf, path);
}

int ConfigFile::numEntries()
{
  return _bcf->num;
}

int ConfigFile::addEntry(const char* entry)
{
  return BCF_Prepend(_bcf, entry);
}

int ConfigFile::insertEntry(const char* entry, int index)
{
  return BCF_Insert(_bcf, entry, index);
}

int ConfigFile::moveEntryDown(int index)
{
  return BCF_MoveDown(_bcf, index);
}

int ConfigFile::moveEntryUp(int index)
{
  return BCF_MoveUp(_bcf, index);
}

const char* ConfigFile::getEntry(int index)
{
  return BCF_GetIndex(_bcf, index);
}

bool ConfigFile::entryExists(const char* entry)
{
  bool rc;
  for(int i = 0; i < BCF_GetNum(_bcf); i++) {
    if(!strcmp(entry, BCF_GetIndex(_bcf, i))) {
      return true;
    }
  }
  return false;
}

int ConfigFile::remove(int index)
{
  return BCF_Remove(_bcf, index);
}

int ConfigFile::rename(const char* newName, int index)
{
  return 0;
}

int ConfigFile::write()
{
  return BCF_Write(_bcf, NULL);
}
