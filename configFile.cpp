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

int ConfigFile::addDongle(const char *entry)
{
  /* First, make sure it is not already in there */
  removeDongle(entry);
  /* Now add it */
  return BCF_PrependDongle(_bcf, entry);
}

int ConfigFile::removeDongle(const char *name)
{
  /* First, we must find the dongle name */
  const char* dongle;
  int i;
  for(i = 0; i < BCF_GetNumDongles(_bcf); i++) {
    dongle = BCF_GetDongle(_bcf, i);
    if(!strcmp(name, dongle)) {
      /* Found it. */
      BCF_RemoveDongle(_bcf, i);
      return 0;
    }
  }
  return -1;
}

const char* ConfigFile::getDongle(int index)
{
  return BCF_GetDongle(_bcf, index);
}

int ConfigFile::write()
{
  return BCF_Write(_bcf, NULL);
}
