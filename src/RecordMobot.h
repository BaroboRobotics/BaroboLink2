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

#pragma once
#include <mobot.h>

enum motionType_e
{
	MOTION_POS,
	MOTION_SLEEP,
};

struct motion_s
{
	enum motionType_e motionType;
	union data_u {
		double sleepDuration;
		double pos[4];
	} data;
	char* name;
};
#if 0
class CRecordMobot :
	public CMobot
{
public:
	CRecordMobot(char* name);
	~CRecordMobot(void);
  /* Override the default connection function to save the address */
  int connectWithAddress(const char address[], int channel);
  const char* getAddress();
	int record();
	int addDelay(double seconds);
	int play(int index);
	int getMotionType(int index);
	int getMotionString(int index, char* buf);
	const char* getMotionName(int index);
	int setMotionName(int index, const char* name);
	int removeMotion(int index, bool releaseData = true);
  int clearAllMotions();
  /* moveMotion:
     Copy motion 'fromindex', insert to 'toindex', delete 'fromindex' */
	int moveMotion(int fromindex, int toindex);
	int numMotions();
private:
	int _numMotions;
	struct motion_s **_motions;
	int _numMotionsAllocated;
	char _name[80];
  char _address[80];
};
#endif

typedef enum recordMobotConnectStatus_e
{
  RMOBOT_NOT_CONNECTED,
  RMOBOT_CONNECTING,
  RMOBOT_CONNECTED
} recordMobotConnectStatus_t;

typedef struct recordMobot_s
{
  mobot_t mobot;
  int numMotions;
  struct motion_s **motions;
  int numMotionsAllocated;
  char name[80];
  char address[80];
  bool bound; /* Is the mobot bound via external TCP socket? */
  int firmwareVersion;
  recordMobotConnectStatus_t connectStatus;
  int dirty;
  int rgb[3];
} recordMobot_t;

#ifdef __cplusplus
extern "C" {
#endif
recordMobot_t* RecordMobot_new();
void RecordMobot_init(recordMobot_t* mobot, const char *name);
void RecordMobot_destroy(recordMobot_t* mobot);
int RecordMobot_connectWithAddress(recordMobot_t* mobot, const char address[], int channel);
const char* RecordMobot_getAddress(recordMobot_t* mobot);
int RecordMobot_record(recordMobot_t* mobot);
int RecordMobot_addDelay(recordMobot_t* mobot, double seconds);
int RecordMobot_isMoving(recordMobot_t* rmobot);
int RecordMobot_play(recordMobot_t* mobot, int index);
int RecordMobot_getMotionType(recordMobot_t* mobot, int index);
int RecordMobot_getChMotionString(recordMobot_t* mobot, int index, char* buf);
int RecordMobot_getChMotionStringB(recordMobot_t* mobot, int index, char* buf);
int RecordMobot_getPythonMotionString(recordMobot_t* mobot, int index, char* buf);
int RecordMobot_getPythonMotionStringB(recordMobot_t* mobot, int index, char* buf);
const char* RecordMobot_getMotionName(recordMobot_t* mobot, int index);
int RecordMobot_setMotionName(recordMobot_t* mobot, int index, const char* name);
int RecordMobot_removeMotion(recordMobot_t* mobot, int index, bool releaseData);
int RecordMobot_clearAllMotions(recordMobot_t* mobot);
int RecordMobot_moveMotion(recordMobot_t* mobot, int fromindex, int toindex);
int RecordMobot_swapMotion(recordMobot_t* mobot, int index1, int index2);
void RecordMobot_setName(recordMobot_t* mobot, const char* name);
recordMobotConnectStatus_t RecordMobot_connectStatus(recordMobot_t* mobot);
#ifdef __cplusplus
}
#endif
