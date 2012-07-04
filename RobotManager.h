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
    bool isConnected(int index);
    bool isPlaying();
    int connectIndex(int index);
    int disconnect(int index);
    recordMobot_t* getUnboundMobot();
    int numConnected();
    int numAvailable();
    void record();
    void addDelay(double seconds);
    void play();
    recordMobot_t* getMobot(int connectIndex);
    string* generateProgram(bool looped = false);
    bool _isPlaying;
  private:
    recordMobot_t *_mobots[MAX_CONNECTED];
    /* _connectAddresses is an array of pointers to 
       ConfigFile::_addresses */
    char *_connectedAddresses[MAX_CONNECTED];
};

#endif
