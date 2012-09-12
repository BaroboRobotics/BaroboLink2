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
    string* generateProgram(bool looped = false);
    bool _isPlaying;
    int _newDndIndex;
  private:
    recordMobot_t *_mobots[MAX_CONNECTED];
    recordMobot_t *_tmpMobot;
    /* _connectAddresses is an array of pointers to 
       ConfigFile::_addresses */
};

#endif
