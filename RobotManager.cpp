#include <stdio.h>
#include <stdlib.h>
#include "RoboMancer.h"
#include "RobotManager.h"
#include "thread_macros.h"

CRobotManager::CRobotManager()
{
  int i;
  for(i = 0; i < MAX_CONNECTED; i++) {
    _mobots[i] = NULL;
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
  if(_mobots[index] == NULL) {
    return false;
  }
  return Mobot_isConnected((mobot_t*)_mobots[index]);
}

int CRobotManager::addEntry(const char* entry)
{
  int rc;
  if(rc = ConfigFile::addEntry(entry)) {
    return rc;
  }

  /* Adjust the array of mobots */
  for(int i = (numEntries()-1); i >= 0; i--) {
    _mobots[i+1] = _mobots[i];
  }
  _mobots[0] = NULL;
}

int CRobotManager::disconnectAll()
{
  int i;
  for(i = 0; i < MAX_CONNECTED; i++) {
    disconnect(i);
  }
  return 0;
}

void CRobotManager::moveMobot(int destIndex, int srcIndex)
{
  _mobots[destIndex] = _mobots[srcIndex];
  _mobots[srcIndex] = NULL;
}

int CRobotManager::moveEntryUp(int index)
{
  int rc;
  if(rc = ConfigFile::moveEntryUp(index)) {
    return rc;
  }
  /* Just swap the data of this entry with the one above it */
  recordMobot_t* tmp;
  tmp = _mobots[index-1];
  _mobots[index-1] = _mobots[index];
  _mobots[index] = tmp;
  return 0;
}

int CRobotManager::moveEntryDown(int index)
{
  int rc;
  if(rc = ConfigFile::moveEntryDown(index)) {
    return rc;
  }
  /* Swap with entry below */
  recordMobot_t *tmp;
  tmp = _mobots[index+1];
  _mobots[index+1] = _mobots[index];
  _mobots[index] = tmp;
  return 0;
}

int CRobotManager::insertEntry(const char* entry, int index)
{
  int rc;
  if(rc = ConfigFile::insertEntry(entry, index)) {
    return rc;
  }
  /* Move the existing mobot array */
  int i;
  for(i = numEntries(); i >= index; i--) {
    _mobots[i+1] = _mobots[i];
  }
  _mobots[index] = NULL;
  return 0;
}

bool CRobotManager::isPlaying()
{
  return _isPlaying;
}

int CRobotManager::connectIndex(int index)
{
  if(isConnected(index)) {
    return 0;
  }
  char name[80];
  sprintf(name, "mobot%d", numConnected()+1);
  int err;
  if(_mobots[index] == NULL) {
    _mobots[index] = RecordMobot_new();
  }
  RecordMobot_init(_mobots[index], name);
  err = RecordMobot_connectWithAddress( _mobots[index], getEntry(index), 1 );
  return err;
}

int CRobotManager::disconnect(int index)
{
  if(_mobots[index] == NULL) {
    return -1;
  }
  /* First check to see how the robot is connected. If it is a TTY connection,
   * we just want to remove the robot from the list of connected robots, but we
   * don't actually want to disconnect. */
  if(((mobot_t*)_mobots[index])->connectionMode != MOBOTCONNECT_TTY) {
    Mobot_disconnect((mobot_t*)_mobots[index]);
  }
  //free(_mobots[index]);
  _mobots[index] = NULL;
  return 0;
}

recordMobot_t* CRobotManager::getUnboundMobot()
{
  int i;
  recordMobot_t* mobot;
  mobot = NULL;
  for(i = 0; i < numEntries(); i++) {
    if(_mobots[i] == NULL) {
      continue;
    }
    if(_mobots[i]->bound == false) {
      mobot = _mobots[i];
      break;
    }
  }
  return mobot;
}

int CRobotManager::numConnected()
{
  int num = 0, i;
  for(i = 0; i < MAX_CONNECTED; i++) {
    if(_mobots[i] == NULL) {
      continue;
    }
    if(_mobots[i] != NULL) {
      num++;
    }
  }
  return num;
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

int CRobotManager::remove(int index)
{
  int rc;
  if(rc = ConfigFile::remove(index)) {
    return rc;
  }
  /* Adjust the list of mobots */
  _tmpMobot = _mobots[index];
  int i;
  for(i = index; i < numEntries(); i++) {
    _mobots[i] = _mobots[i+1];
  }
  return 0;
}

void CRobotManager::restoreSavedMobot(int index)
{
  if(_mobots[index] != NULL) {
    free(_mobots[index]);
  }
  _mobots[index] = _tmpMobot;
  _tmpMobot = NULL;
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
  int i;
  for(i = 0; i <= MAX_CONNECTED ; i++ ) {
    if(_mobots[i] == NULL) {continue;}
    if((_mobots[i] != NULL) && (_mobots[i]->connectStatus == RMOBOT_CONNECTED)) {
      connectIndex--;
    }
    if(connectIndex < 0) {
      break;
    }
  }
	return _mobots[i];
}

recordMobot_t* CRobotManager::getMobotIndex(int index)
{
  return _mobots[index];
}

string* CRobotManager::generateProgram(bool looped, bool holdOnExit)
{
  string buf;
  string *program = new string();
  char tbuf[200];
  int i, j;
  char tmp[80];
  *program += "/* Program generated by RoboMancer */\n";
  *program += "#include <mobot.h>\n\n";

  //*program += "int main() {\n";

  /* Declare the appropiate number of CMobot variables */
  for(i = 0; i < numConnected(); i++) {
    if(getMobot(i)->mobot.formFactor == MOBOTFORM_I) {
      sprintf(tbuf, "CMobotI mobot%d;\n", i+1);
    } else if (getMobot(i)->mobot.formFactor == MOBOTFORM_L) {
      sprintf(tbuf, "CMobotL mobot%d;\n", i+1);
    } else {
      sprintf(tbuf, "CMobot mobot%d;\n", i+1);
    }
    buf.assign(tbuf);
    *program += buf;
  }

  /* Connect to each one */
  for(i = 0; i < numConnected(); i++) {
    if(getMobot(i)) {
    }
    sprintf(tbuf, "mobot%d.connect();\n", i+1);
    buf.assign(tbuf);
    *program += buf;
  }

  if(looped) {
    *program += "/* Set the \"num\" variable to the number of times to loop. */\n";
    *program += "int num = 3;\n";
    *program += "int i;\n";
    *program += "for(i = 0; i < num; i++) {";
  }
  /* Make sure there were connected mobots */
  if(getMobot(0) != NULL) {
    /* Set all of the robot names so they are in order */
    for(j = 0; j < numConnected(); j++) {
      sprintf(tmp, "mobot%d", j+1);
      RecordMobot_setName(getMobot(j), tmp);
    }
    /* Now go through each motion */
    for(i = 0; i < getMobot(0)->numMotions; i++) {
      *program += "\n";
      /* First, print the comment for the motion */
      sprintf(tbuf, "/* %s */\n", RecordMobot_getMotionName(getMobot(0), i));
      buf.assign(tbuf);
      //*program += "    ";
      if(looped) *program += "    ";
      *program += buf;
      /* Now, print each robots motion */
      for(j = 0; j < numConnected(); j++) {
        if(numConnected() == 1) {
          RecordMobot_getMotionStringB(getMobot(j), i, tbuf);
        } else {
          RecordMobot_getMotionString(getMobot(j), i, tbuf);
        }
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
        if(numConnected() > 1) {
          sprintf(tbuf, "mobot%d.moveWait();\n", j+1);
          buf.assign(tbuf);
          if(looped) *program += "    ";
          *program += buf;
        }
      }
    }
  }
  if(looped) {
    *program += "}\n";
  }

  if(holdOnExit) {
    for(i = 0; i < numConnected(); i++) {
      sprintf(tbuf, "mobot%d.setExitState(MOBOT_HOLD);\n", i+1);
      buf.assign(tbuf);
      *program += buf;
    }
  }

  //*program += "return 0;\n";
  //*program += "}\n";
  *program += "\n";
  return program;
}
