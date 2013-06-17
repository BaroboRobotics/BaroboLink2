/*
   Copyright 2013 Barobo, Inc.

   This file is part of BaroboLink.

   BaroboLink is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   BaroboLink is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with BaroboLink.  If not, see <http://www.gnu.org/licenses/>.
*/

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


