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

#ifndef _ROBOT_MANAGER_H_
#define _ROBOT_MANAGER_H_

#include <string.h>
#include <string>

#include "configFile.h"
#include <mobot.h>
#include "RecordMobot.h"

#define MAX_CONNECTED 100
using namespace std;

class CRobotManager : public ConfigFile
{
  public:
    CRobotManager();
    ~CRobotManager();
    int addEntry(const char* entry);
    void moveMobot(int destIndex, int srcIndex);
    int moveEntryUp(int index);
    int moveEntryDown(int index);
    int insertEntry(const char* entry, int index);
    bool isConnected(int index);
    bool isPlaying();
    int connectIndex(int index);
    int disconnect(int index);
    int disconnectAll();
    recordMobot_t* getUnboundMobot();
    int numConnected();
    int numAvailable();
    void record();
    int remove(int index);
    void restoreSavedMobot(int index);
    void addDelay(double seconds);
    void play();
    recordMobot_t* getMobot(int connectIndex);
    recordMobot_t* getMobotIndex(int index);
    string* generateChProgram(bool looped = false, bool holdOnExit = false);
    string* generateCppProgram(bool looped = false, bool holdOnExit = false);
    string* generatePythonProgram(bool looped = false, bool holdOnExit = false);
    bool _isPlaying;
    int _newDndIndex;
  private:
    recordMobot_t *_mobots[MAX_CONNECTED];
    recordMobot_t *_tmpMobot;
    /* _connectAddresses is an array of pointers to 
       ConfigFile::_addresses */
};

#endif
