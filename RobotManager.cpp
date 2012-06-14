#include <stdio.h>
#include <stdlib.h>
#include "RobotManager.h"
#include "thread_macros.h"

CRobotManager::CRobotManager()
{
  int i;
  for(i = 0; i < MAX_CONNECTED; i++) {
    _connected[i] = false;
    _mobots[i] = NULL;
    _connectedAddresses[i] = NULL;
  }
  _isPlaying = false;
}

CRobotManager::~CRobotManager()
{
}

bool CRobotManager::isConnected(int index) 
{
  if((index >= numEntries()) || index < 0) {
    return false;
  }
  return _connected[index];
}

bool CRobotManager::isPlaying()
{
  return _isPlaying;
}

void CRobotManager::setConnected(int index, bool connected)
{
  _connected[index] = connected;
}

int CRobotManager::connect(int availableIndex)
{
  int i;
  int index;
  int err = 0;
  char name[80];
  sprintf(name, "robot%d", numConnected()+1);
  recordMobot_t *mobot = (recordMobot_t*)malloc(sizeof(recordMobot_t));
  RecordMobot_init(mobot, name);
  index = availableIndexToIndex(availableIndex);
  if(err = Mobot_connectWithAddress( (mobot_t*)mobot, getEntry(index), 1 )) {
    return err;
  }
  /* Enable the button callback */
  /* FIXME */
  //mobot->enableButtonCallback(CDialogTeaching::OnMobotButton);
  Mobot_setJointSpeedRatios((mobot_t*)mobot, 1, 1, 1, 1);
  /* Insert the newly connected robot to the bottom of the list. */
  _mobots[numConnected()] = mobot;
  _connectedAddresses[numConnected()] = 
	  _addresses[availableIndexToIndex(availableIndex)];
  _connected[availableIndexToIndex(availableIndex)] = true;
  return err;
}

int CRobotManager::connectIndex(int index)
{
  if(isConnected(index)) {
    return 0;
  }
  char name[80];
  sprintf(name, "robot%d", numConnected()+1);
  recordMobot_t *mobot = (recordMobot_t*)malloc(sizeof(recordMobot_t));
  RecordMobot_init(mobot, name);
  int err;
  if(err = Mobot_connectWithAddress( (mobot_t*)mobot, getEntry(index), 1 )) {
    return err;
  }
  Mobot_setJointSpeedRatios((mobot_t*)mobot, 1, 1, 1, 1);
  /* Insert the newly connected robot to the bottom of the list. */
  _mobots[numConnected()] = mobot;
  _connectedAddresses[numConnected()] = 
	  _addresses[index];
  _connected[index] = true;
  return err;
}

int CRobotManager::disconnect(int connectIndex)
{
  int i;
  int foundEntry = 0;
  /* Need to find the ConfigFile index of the connected robot */
  for(i = 0; i < numEntries(); i++) {
    if(_connectedAddresses[connectIndex] == _addresses[i]) {
      foundEntry = 1;
      break;
    }
  }
  if(foundEntry == 0) { return -1; }
  Mobot_disconnect((mobot_t*)_mobots[connectIndex]);
  /* Need to shift addresses and mobots up */
  int j;
  for(j = connectIndex+1; j < numConnected(); j++) {
    _connectedAddresses[j-1] = _connectedAddresses[j];
    _mobots[j-1] = _mobots[j];
  }
  _connected[i] = false;
  return 0; 
}

int CRobotManager::moveUp(int connectIndex) {
  recordMobot_t* tempMobot;
  char* tempAddr;
  if(connectIndex < 1 || connectIndex >= numConnected()) return -1;
  /* Swap the robot with the one prior */
  tempMobot = _mobots[connectIndex-1];
  tempAddr = _connectedAddresses[connectIndex-1];
  _mobots[connectIndex-1] = _mobots[connectIndex];
  _connectedAddresses[connectIndex-1] = _connectedAddresses[connectIndex];
  _mobots[connectIndex] = tempMobot;
  _connectedAddresses[connectIndex] = tempAddr;
  return 0;
}

int CRobotManager::moveDown(int connectIndex) {
  recordMobot_t* tempMobot;
  char* tempAddr;
  if(connectIndex < 0 || connectIndex >= (numConnected()-1)) return -1;
  tempMobot = _mobots[connectIndex + 1];
  tempAddr = _connectedAddresses[connectIndex + 1];
  _mobots[connectIndex+1] = _mobots[connectIndex];
  _connectedAddresses[connectIndex+1] = _connectedAddresses[connectIndex];
  _mobots[connectIndex] = tempMobot;
  _connectedAddresses[connectIndex] = tempAddr;
  return 0;
}

int CRobotManager::numConnected()
{
  int num = 0, i;
  for(i = 0; i < numEntries(); i++) {
    if(_connected[i]) {
      num++;
    }
  }
  return num;
}

const char* CRobotManager::getConnected(int connectIndex) {
	if(connectIndex < 0 || connectIndex >= numConnected()) {
		return NULL;
	}
	return _connectedAddresses[connectIndex];
}

int CRobotManager::availableIndexToIndex(int availableIndex)
{
	int index = 0;
	int i;
	if(availableIndex < 0 || availableIndex > numAvailable()) {
		return -1;
	}
	for(index = 0, i = 0; i <= availableIndex; index++) {
		if(_connected[index] == false) {
			i++;
		}
	}
	index--;
	return index;
}

int CRobotManager::indexToConnectIndex(int index)
{
  if(_connected[index] == false) {
    return -1;
  }
  int i, ci=0;
  for(i = 0; i < index; i++) {
    if(_connected[i]) {
      ci++;
    }
  }
  return ci;
}

int CRobotManager::numAvailable()
{
	return numEntries() - numConnected();
}

void CRobotManager::record()
{
  int i;
  recordMobot_t* mobot;
  for(i = 0; i < numConnected(); i++) {
    mobot = getMobot(i);
    RecordMobot_record(mobot);
  }
}

void CRobotManager::addDelay(double seconds)
{
  int i;
  recordMobot_t* mobot;
  for(i = 0; i < numConnected(); i++) {
    mobot = getMobot(i);
    RecordMobot_addDelay(mobot, seconds);
  }
}

void* robotManagerPlayThread(void* arg)
{
  CRobotManager *rm = (CRobotManager*)arg;
  recordMobot_t* mobot;
  if(rm->numConnected() <= 0) {
    rm->_isPlaying = false;
    return NULL;
  }
  int index, i;
  /* Go through each motion */
  for(index = 0; index < rm->getMobot(0)->numMotions; index++) {
    /* Go through each mobot */
    for(i = 0; i < rm->numConnected(); i++) {
      mobot = rm->getMobot(i);
      if(RecordMobot_getMotionType(mobot, index) == MOTION_SLEEP) {
        /* Sleep the correct amount of time and break */
        RecordMobot_play(mobot, index);
        break;
      } else if (RecordMobot_getMotionType(mobot, index) == MOTION_POS) {
        RecordMobot_play(mobot, index);
      } else {
        fprintf(stderr, "MEMORY ERROR %s:%d\n", __FILE__, __LINE__);
        rm->_isPlaying = false;
        return NULL;
      }
    }
  }
  rm->_isPlaying = false;
}

void CRobotManager::play()
{
  _isPlaying = true;
  THREAD_T thread;
  THREAD_CREATE(&thread, robotManagerPlayThread, this);
}

recordMobot_t* CRobotManager::getMobot(int connectIndex)
{
	if(connectIndex < 0 || connectIndex >= numConnected()) {
		return NULL;
	}
	return _mobots[connectIndex];
}

string* CRobotManager::generateProgram(bool looped)
{
  string buf;
  string *program = new string();
  char tbuf[200];
  int i, j;
  *program += "/* Program generated by RobotController */\n";
  *program += "#include <mobot.h>\n\n";

  /* Declare the appropiate number of CMobot variables */
  for(i = 0; i < numConnected(); i++) {
    sprintf(tbuf, "CMobot robot%d;\n", i+1);
    buf.assign(tbuf);
    *program += buf;
  }

  /* Connect to each one */
  for(i = 0; i < numConnected(); i++) {
    sprintf(tbuf, "robot%d.connectWithAddress(\"%s\", 1);\n", i+1, 
        RecordMobot_getAddress(getMobot(i)));
    buf.assign(tbuf);
    *program += buf;
  }

  if(looped) {
    *program += "/* Set the \"num\" variable to the number of times to loop. */\n";
    *program += "int num = 3;\n";
    *program += "int i;\n";
    *program += "for(i = 0; i < num; i++) {";
  }
  /* Now go through each motion */
  for(i = 0; i < getMobot(0)->numMotions; i++) {
    *program += "\n";
    /* First, print the comment for the motion */
    sprintf(tbuf, "/* %s */\n", RecordMobot_getMotionName(getMobot(0), i));
    buf.assign(tbuf);
    if(looped) *program += "    ";
    *program += buf;
    /* Now, print each robots motion */
    for(j = 0; j < numConnected(); j++) {
      RecordMobot_getMotionString(getMobot(j), i, tbuf);
      buf.assign(tbuf);
      buf += "\n";
      if(looped) *program += "    ";
      *program += buf;
      if(RecordMobot_getMotionType(getMobot(j), i) == MOTION_SLEEP) {
        break;
      }
    }
    /* Make sure all the robots are done moving */
    for(j = 0; j < numConnected(); j++) {
      if(RecordMobot_getMotionType(getMobot(j), i) == MOTION_SLEEP) {
        break;
      }
      sprintf(tbuf, "robot%d.moveWait();\n", j+1);
      buf.assign(tbuf);
      if(looped) *program += "    ";
      *program += buf;
    }
  }
  if(looped) {
    *program += "}\n";
  }
  return program;
}
