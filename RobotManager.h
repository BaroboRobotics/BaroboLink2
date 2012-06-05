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
    void setConnected(int index, bool connected);
    int connect(int availableIndex);
    int connectIndex(int index);
    int disconnect(int connectIndex);
    int moveUp(int connectIndex);
    int moveDown(int connectIndex);
    int numConnected();
    const char* getConnected(int connectIndex);
    int availableIndexToIndex(int index);
    int numAvailable();
    recordMobot_t* getMobot(int connectIndex);
    string* generateProgram(bool looped = false);
  private:
    bool _connected[MAX_CONNECTED]; /* Index by ConfigFile */
    recordMobot_t *_mobots[MAX_CONNECTED];
    /* _connectAddresses is an array of pointers to 
       ConfigFile::_addresses */
    char *_connectedAddresses[MAX_CONNECTED];
};

#endif
