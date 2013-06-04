/*
   Copyright 2013 Barobo, Inc.

   This file is part of BaroboLink.

   Foobar is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Foobar is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <windows.h>
#undef NONRELEASE
#include "mobot.h"
#undef THREAD_T
#undef MUTEX_T
#undef COND_T
#define RAD2DEG(x) ((x)*180.0/M_PI)
#define DEG2RAD(x) ((x)*M_PI/180.0)

/* ******* *
 * THREADS *
 * ******* */
#ifndef THREAD_T
#define THREAD_T HANDLE
#endif

#define pthread_t HANDLE

#define THREAD_CREATE(thread_handle, function, arg) \
  *(thread_handle) = CreateThread( \
      NULL, \
      0, \
      (LPTHREAD_START_ROUTINE)function, \
      arg, \
      0, \
      NULL \
      )

#define MUTEX_T HANDLE
#define COND_T HANDLE

CMobot::CMobot()
{
  _comms = (mobot_t*)malloc(sizeof(mobot_t));
  Mobot_init(_comms);
}

CMobot::~CMobot()
{
  stopAllJoints();
  if(_comms->connected) {
    disconnect();
  }
}

int CMobot::blinkLED(double delay, int numBlinks)
{
  return Mobot_blinkLED(_comms, delay, numBlinks);
}

int CMobot::connect()
{
  return Mobot_connect(_comms);
}

int CMobot::connectWithAddress(const char* address, int channel)
{
  return Mobot_connectWithAddress(_comms, address, channel);
}

int CMobot::disconnect()
{
  return Mobot_disconnect(_comms);
}

int CMobot::enableButtonCallback(void (*buttonCallback)(CMobot* robot, int button, int buttonDown))
{
  return Mobot_enableButtonCallback(
      _comms,
      this,
      (void(*)(void*,int,int))buttonCallback);
}

int CMobot::disableButtonCallback()
{
  return Mobot_disableButtonCallback(_comms);
}

int CMobot::isConnected()
{
  return Mobot_isConnected(_comms);
}

int CMobot::isMoving()
{
  return Mobot_isMoving(_comms);
}

const char* CMobot::getConfigFilePath()
{
  return Mobot_getConfigFilePath();
}

int CMobot::getJointAngle(robotJointId_t id, double &angle)
{
  int err;
  err = Mobot_getJointAngle(_comms, id, &angle);
  angle = RAD2DEG(angle);
  return err;
}

int CMobot::getJointAngles(
    double &angle1,
    double &angle2,
    double &angle3,
    double &angle4)
{
  double time;
  int err;
  err = Mobot_getJointAnglesTime(
      _comms, 
      &time,
      &angle1,
      &angle2,
      &angle3,
      &angle4);
  if(err) return err;
  angle1 = RAD2DEG(angle1);
  angle2 = RAD2DEG(angle2);
  angle3 = RAD2DEG(angle3);
  angle4 = RAD2DEG(angle4);
  return 0;
}

int CMobot::getJointDirection(robotJointId_t id, mobotJointState_t &dir)
{
  return Mobot_getJointDirection(_comms, id, &dir);
}

int CMobot::getJointMaxSpeed(robotJointId_t id, double &maxSpeed)
{
  int err = Mobot_getJointMaxSpeed(_comms, id, &maxSpeed);
  maxSpeed = RAD2DEG(maxSpeed);
  return err;
}

int CMobot::getJointSafetyAngle(double &angle)
{
  int r;
  double a;
  r = Mobot_getJointSafetyAngle(_comms, &a);
  angle = RAD2DEG(a);
  return r;
}

int CMobot::getJointSafetyAngleTimeout(double &seconds)
{
  return Mobot_getJointSafetyAngleTimeout(_comms, &seconds);
}

int CMobot::getJointSpeed(robotJointId_t id, double &speed)
{
  int err;
  err = Mobot_getJointSpeed(_comms, id, &speed);
  speed = RAD2DEG(speed);
  return err;
}

int CMobot::getJointSpeedRatio(robotJointId_t id, double &ratio)
{
  return Mobot_getJointSpeedRatio(_comms, id, &ratio);
}

int CMobot::getJointSpeeds(double &speed1, double &speed2, double &speed3, double &speed4)
{
  int i;
  int err = Mobot_getJointSpeeds(_comms, &speed1, &speed2, &speed3, &speed4);
  speed1 = RAD2DEG(speed1);
  speed2 = RAD2DEG(speed2);
  speed3 = RAD2DEG(speed3);
  speed4 = RAD2DEG(speed4);
  return err;
}

int CMobot::getJointSpeedRatios(double &ratio1, double &ratio2, double &ratio3, double &ratio4)
{
  return Mobot_getJointSpeedRatios(_comms, &ratio1, &ratio2, &ratio3, &ratio4);
}

int CMobot::getJointState(robotJointId_t id, mobotJointState_t &state)
{
  return Mobot_getJointState(_comms, id, &state);
}

mobot_t* CMobot::getMobotObject()
{
  return _comms;
}

int CMobot::move( double angle1,
                        double angle2,
                        double angle3,
                        double angle4)
{
  return Mobot_move(
      _comms, 
      DEG2RAD(angle1), 
      DEG2RAD(angle2), 
      DEG2RAD(angle3), 
      DEG2RAD(angle4));
}

int CMobot::moveNB( double angle1,
                        double angle2,
                        double angle3,
                        double angle4)
{
  return Mobot_moveNB(
      _comms, 
      DEG2RAD(angle1), 
      DEG2RAD(angle2), 
      DEG2RAD(angle3), 
      DEG2RAD(angle4));
}

int CMobot::moveContinuousNB( mobotJointState_t dir1, mobotJointState_t dir2, mobotJointState_t dir3, mobotJointState_t dir4)
{
  return Mobot_moveContinuousNB(_comms, dir1, dir2, dir3, dir4);
}

int CMobot::moveContinuousTime( mobotJointState_t dir1, mobotJointState_t dir2, mobotJointState_t dir3, mobotJointState_t dir4, double seconds)
{
  return Mobot_moveContinuousTime(_comms, dir1, dir2, dir3, dir4, seconds);
}

int CMobot::moveJointContinuousNB(robotJointId_t id, mobotJointState_t dir)
{
  return Mobot_moveJointContinuousNB(_comms, id, dir);
}

int CMobot::moveJointContinuousTime(robotJointId_t id, mobotJointState_t dir, double seconds)
{
  return Mobot_moveJointContinuousTime(_comms, id, dir, seconds);
}

int CMobot::moveJoint(robotJointId_t id, double angle)
{
  return Mobot_moveJoint(_comms, id, DEG2RAD(angle));
}

int CMobot::moveJointNB(robotJointId_t id, double angle)
{
  return Mobot_moveJointNB(_comms, id, DEG2RAD(angle));
}

int CMobot::moveJointTo(robotJointId_t id, double angle)
{
  return Mobot_moveJointTo(_comms, id, DEG2RAD(angle));
}

int CMobot::moveJointToDirect(robotJointId_t id, double angle)
{
  return Mobot_moveJointToDirect(_comms, id, DEG2RAD(angle));
}

int CMobot::moveJointToNB(robotJointId_t id, double angle)
{
  return Mobot_moveJointToNB(_comms, id, DEG2RAD(angle));
}

int CMobot::moveJointToDirectNB(robotJointId_t id, double angle)
{
  return Mobot_moveJointToDirectNB(_comms, id, DEG2RAD(angle));
}

int CMobot::driveJointToDirectNB(robotJointId_t id, double angle)
{
  return Mobot_driveJointToDirectNB(_comms, id, DEG2RAD(angle));
}

int CMobot::moveJointWait(robotJointId_t id)
{
  return Mobot_moveJointWait(_comms, id);
}

int CMobot::moveTo( double angle1,
                          double angle2,
                          double angle3,
                          double angle4)
{
  return Mobot_moveTo(
      _comms, 
      DEG2RAD(angle1), 
      DEG2RAD(angle2), 
      DEG2RAD(angle3), 
      DEG2RAD(angle4));
}

int CMobot::moveToDirect( double angle1,
                          double angle2,
                          double angle3,
                          double angle4)
{
  return Mobot_moveToDirect(
      _comms, 
      DEG2RAD(angle1), 
      DEG2RAD(angle2), 
      DEG2RAD(angle3), 
      DEG2RAD(angle4));
}

int CMobot::driveToDirect( double angle1,
                          double angle2,
                          double angle3,
                          double angle4)
{
  return Mobot_driveToDirect(
      _comms, 
      DEG2RAD(angle1), 
      DEG2RAD(angle2), 
      DEG2RAD(angle3), 
      DEG2RAD(angle4));
}

int CMobot::moveToNB( double angle1,
                          double angle2,
                          double angle3,
                          double angle4)
{
  return Mobot_moveToNB(
      _comms, 
      DEG2RAD(angle1), 
      DEG2RAD(angle2), 
      DEG2RAD(angle3), 
      DEG2RAD(angle4));
}

int CMobot::moveToDirectNB( double angle1,
                          double angle2,
                          double angle3,
                          double angle4)
{
  return Mobot_moveToDirectNB(
      _comms, 
      DEG2RAD(angle1), 
      DEG2RAD(angle2), 
      DEG2RAD(angle3), 
      DEG2RAD(angle4));
}

int CMobot::driveToDirectNB( double angle1,
                          double angle2,
                          double angle3,
                          double angle4)
{
  return Mobot_driveToDirectNB(
      _comms, 
      DEG2RAD(angle1), 
      DEG2RAD(angle2), 
      DEG2RAD(angle3), 
      DEG2RAD(angle4));
}

int CMobot::moveWait()
{
  return Mobot_moveWait(_comms);
}

int CMobot::moveToZero()
{
  return Mobot_moveToZero(_comms);
}

int CMobot::moveToZeroNB()
{
  return Mobot_moveToZeroNB(_comms);
}

int CMobot::recordAngle(robotJointId_t id, double* time, double* angle, int num, double seconds)
{
  return Mobot_recordAngle(_comms, id, time, angle, num, seconds);
}

int CMobot::recordAngles(double *time, 
    double *angle1, 
    double *angle2, 
    double *angle3, 
    double *angle4, 
    int num, 
    double seconds)
{
  return Mobot_recordAngles(_comms, time, angle1, angle2, angle3, angle4, num, seconds);
}

int CMobot::recordWait()
{
  return Mobot_recordWait(_comms);
}

int CMobot::setJointSafetyAngle(double angle)
{
  angle = DEG2RAD(angle);
  return Mobot_setJointSafetyAngle(_comms, angle);
}

int CMobot::setJointSafetyAngleTimeout(double seconds)
{
  return Mobot_setJointSafetyAngleTimeout(_comms, seconds);
}

int CMobot::setJointDirection(robotJointId_t id, mobotJointState_t dir)
{
  return Mobot_setJointDirection(_comms, id, dir);
}

int CMobot::setJointSpeed(robotJointId_t id, double speed)
{
  return Mobot_setJointSpeed(_comms, id, DEG2RAD(speed));
}

int CMobot::setJointSpeeds(double speed1, double speed2, double speed3, double speed4)
{
  return Mobot_setJointSpeeds(
      _comms, 
      DEG2RAD(speed1), 
      DEG2RAD(speed2), 
      DEG2RAD(speed3), 
      DEG2RAD(speed4));
}

int CMobot::setJointSpeedRatio(robotJointId_t id, double ratio)
{
  return Mobot_setJointSpeedRatio(_comms, id, ratio);
}

int CMobot::setJointSpeedRatios(double ratio1, double ratio2, double ratio3, double ratio4)
{
  return Mobot_setJointSpeedRatios(_comms, ratio1, ratio2, ratio3, ratio4);
}

int CMobot::setMotorPower(robotJointId_t id, int power)
{
  return Mobot_setMotorPower(_comms, id, power);
}

int CMobot::setTwoWheelRobotSpeed(double speed, double radius)
{
  return Mobot_setTwoWheelRobotSpeed(_comms, speed, radius);
}

int CMobot::stopAllJoints()
{
  return Mobot_stopAllJoints(_comms);
}

int CMobot::motionArch(double angle)
{
  return Mobot_motionArch(_comms, DEG2RAD(angle));
}

int CMobot::motionArchNB(double angle)
{
  return Mobot_motionArchNB(_comms, DEG2RAD(angle));
}

int CMobot::motionInchwormLeft(int num)
{
  return Mobot_motionInchwormLeft(_comms, num);
}

int CMobot::motionInchwormLeftNB(int num)
{
  return Mobot_motionInchwormLeftNB(_comms, num);
}

int CMobot::motionInchwormRight(int num)
{
  return Mobot_motionInchwormRight(_comms, num);
}

int CMobot::motionInchwormRightNB(int num)
{
  return Mobot_motionInchwormRightNB(_comms, num);
}

int CMobot::motionRollBackward(double angle)
{
  return Mobot_motionRollBackward(_comms, DEG2RAD(angle));
}

int CMobot::motionRollBackwardNB(double angle)
{
  return Mobot_motionRollBackwardNB(_comms, DEG2RAD(angle));
}

int CMobot::motionRollForward(double angle)
{
  return Mobot_motionRollForward(_comms, DEG2RAD(angle));
}

int CMobot::motionRollForwardNB(double angle)
{
  return Mobot_motionRollForwardNB(_comms, DEG2RAD(angle));
}

int CMobot::motionSkinny(double angle)
{
  return Mobot_motionSkinny(_comms, DEG2RAD(angle));
}

int CMobot::motionSkinnyNB(double angle)
{
  return Mobot_motionSkinnyNB(_comms, DEG2RAD(angle));
}

int CMobot::motionStand()
{
  return Mobot_motionStand(_comms);
}

int CMobot::motionStandNB()
{
  return Mobot_motionStandNB(_comms);
}

int CMobot::motionTumbleRight(int num)
{
  return Mobot_motionTumbleRight(_comms, num);
}

int CMobot::motionTumbleRightNB(int num)
{
  return Mobot_motionTumbleRightNB(_comms, num);
}

int CMobot::motionTumbleLeft(int num)
{
  return Mobot_motionTumbleLeft(_comms, num);
}

int CMobot::motionTumbleLeftNB(int num)
{
  return Mobot_motionTumbleLeftNB(_comms, num);
}

int CMobot::motionTurnLeft(double angle)
{
  return Mobot_motionTurnLeft(_comms, DEG2RAD(angle));
}

int CMobot::motionTurnLeftNB(double angle)
{
  return Mobot_motionTurnLeftNB(_comms, DEG2RAD(angle));
}

int CMobot::motionTurnRight(double angle)
{
  return Mobot_motionTurnRight(_comms, DEG2RAD(angle));
}

int CMobot::motionTurnRightNB(double angle)
{
  return Mobot_motionTurnRightNB(_comms, DEG2RAD(angle));
}

int CMobot::motionUnstand()
{
  return Mobot_motionUnstand(_comms);
}

int CMobot::motionUnstandNB()
{
  return Mobot_motionUnstandNB(_comms);
}

int CMobot::motionWait()
{
  return Mobot_motionWait(_comms);
}

CMobotGroup::CMobotGroup()
{
  _numRobots = 0;
  _motionInProgress = 0;
  _thread = (THREAD_T*)malloc(sizeof(THREAD_T));
}

CMobotGroup::~CMobotGroup()
{
}

int CMobotGroup::addRobot(CMobot& robot)
{
  _robots[_numRobots] = &robot;
  _numRobots++;
  return 0;
}

int CMobotGroup::isMoving()
{
  for(int i = 0; i < _numRobots; i++) {
    if(_robots[i]->isMoving()) {
      return 1;
    }
  }
  return 0;
}

int CMobotGroup::move(double angle1, double angle2, double angle3, double angle4)
{
  moveNB(angle1, angle2, angle3, angle4);
  return moveWait();
}

int CMobotGroup::moveNB(double angle1, double angle2, double angle3, double angle4)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveNB(angle1, angle2, angle3, angle4);
  }
  return 0;
} 

int CMobotGroup::moveContinuousNB(mobotJointState_t dir1, 
                       mobotJointState_t dir2, 
                       mobotJointState_t dir3, 
                       mobotJointState_t dir4)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveContinuousNB(dir1, dir2, dir3, dir4);
  }
  return 0;
}

int CMobotGroup::moveContinuousTime(mobotJointState_t dir1, 
                           mobotJointState_t dir2, 
                           mobotJointState_t dir3, 
                           mobotJointState_t dir4, 
                           double seconds)
{
  int msecs = seconds * 1000.0;
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveContinuousNB(dir1, dir2, dir3, dir4);
  }
#ifdef _WIN32
  Sleep(msecs);
#else
  usleep(msecs*1000);
#endif
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->stopAllJoints();
  }
  return 0;
}

int CMobotGroup::moveJointContinuousNB(robotJointId_t id, mobotJointState_t dir)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveJointContinuousNB(id, dir);
  }
  return 0;
}

int CMobotGroup::moveJointContinuousTime(robotJointId_t id, mobotJointState_t dir, double seconds)
{
  int msecs = seconds * 1000.0;
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveJointContinuousNB(id, dir);
  }
#ifdef _WIN32
  Sleep(msecs);
#else
  usleep(msecs * 1000);
#endif
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->stopAllJoints();
  }
  return 0;
}

int CMobotGroup::moveJointTo(robotJointId_t id, double angle)
{
  moveJointToNB(id, angle);
  return moveWait();
}

int CMobotGroup::moveJointToNB(robotJointId_t id, double angle)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveJointToNB(id, angle);
  }
  return 0;
}

int CMobotGroup::moveJointWait(robotJointId_t id)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveJointWait(id);
  }
  return 0;
}

int CMobotGroup::moveTo(double angle1, double angle2, double angle3, double angle4)
{
  moveToNB(angle1, angle2, angle3, angle4);
  return moveWait();
}

int CMobotGroup::moveToNB(double angle1, double angle2, double angle3, double angle4)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveToNB(angle1, angle2, angle3, angle4);
  }
  return 0;
}

int CMobotGroup::moveWait()
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveWait();
  }
  return 0;
}

int CMobotGroup::moveToZeroNB()
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->moveToZeroNB();
  }
  return 0;
}

int CMobotGroup::moveToZero()
{
  moveToZeroNB();
  return moveWait();
}

int CMobotGroup::setJointSpeed(robotJointId_t id, double speed)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->setJointSpeed(id, speed);
  }
  return 0;
}

int CMobotGroup::setJointSpeeds(double speed1, double speed2, double speed3, double speed4)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->setJointSpeeds(speed1, speed2, speed3, speed4);
  }
  return 0;
}

int CMobotGroup::setJointSpeedRatio(robotJointId_t id, double ratio)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->setJointSpeedRatio(id, ratio);
  }
  return 0;
}

int CMobotGroup::setJointSpeedRatios(double ratio1, double ratio2, double ratio3, double ratio4)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->setJointSpeedRatios(ratio1, ratio2, ratio3, ratio4);
  }
  return 0;
}

int CMobotGroup::setTwoWheelRobotSpeed(double speed, double radius)
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->setTwoWheelRobotSpeed(speed, radius);
  }
  return 0;
}

int CMobotGroup::stopAllJoints()
{
  for(int i = 0; i < _numRobots; i++) {
    _robots[i]->stopAllJoints();
  }
  return 0;
}

int CMobotGroup::motionArch(double angle) {
  argDouble = angle;
  _motionInProgress++;
  motionArchThread(this);
  return 0;
}

int CMobotGroup::motionArchNB(double angle) {
  argDouble = angle;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionArchThread, this);
  return 0;
}

void* CMobotGroup::motionArchThread(void* arg) 
{
  CMobotGroup *cmg = (CMobotGroup*)arg;
  cmg->moveJointToNB(ROBOT_JOINT2, -cmg->argDouble/2);
  cmg->moveJointToNB(ROBOT_JOINT3, cmg->argDouble/2);
  cmg->moveJointWait(ROBOT_JOINT2);
  cmg->moveJointWait(ROBOT_JOINT3);
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionInchwormLeft(int num)
{
  argInt = num;
  _motionInProgress++;
  motionInchwormLeftThread(this);
  return 0;
}

int CMobotGroup::motionInchwormLeftNB(int num)
{
  argInt = num;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionInchwormLeftThread, this);
  return 0;
}

void* CMobotGroup::motionInchwormLeftThread(void* arg)
{
  int i;
  CMobotGroup *cmg = (CMobotGroup*)arg;
  cmg->moveJointToNB(ROBOT_JOINT2, 0);
  cmg->moveJointToNB(ROBOT_JOINT3, 0);
  cmg->moveWait();
  for(i = 0; i < cmg->argInt ; i++) {
    cmg->moveJointTo(ROBOT_JOINT2, -50);
    cmg->moveJointTo(ROBOT_JOINT3, 50);
    cmg->moveJointTo(ROBOT_JOINT2, 0);
    cmg->moveJointTo(ROBOT_JOINT3, 0);
  }
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionInchwormRight(int num)
{
  argInt = num;
  _motionInProgress++;
  motionInchwormRightThread(this);
  return 0;
}

int CMobotGroup::motionInchwormRightNB(int num)
{
  argInt = num;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionInchwormRightThread, this);
  return 0;
}

void* CMobotGroup::motionInchwormRightThread(void* arg)
{
  int i;
  CMobotGroup *cmg = (CMobotGroup*)arg;

  cmg->moveJointToNB(ROBOT_JOINT2, 0);
  cmg->moveJointToNB(ROBOT_JOINT3, 0);
  cmg->moveWait();
  for(i = 0; i < cmg->argInt; i++) {
    cmg->moveJointTo(ROBOT_JOINT3, 50);
    cmg->moveJointTo(ROBOT_JOINT2, -50);
    cmg->moveJointTo(ROBOT_JOINT3, 0);
    cmg->moveJointTo(ROBOT_JOINT2, 0);
  }
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionRollBackward(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  motionRollBackwardThread(this);
  return 0;
}

int CMobotGroup::motionRollBackwardNB(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionRollBackwardThread, this);
  return 0;
}

void* CMobotGroup::motionRollBackwardThread(void* arg)
{
  CMobotGroup *cmg = (CMobotGroup*)arg;
  cmg->move(-cmg->argDouble, 0, 0, -cmg->argDouble);
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionRollForward(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  motionRollForwardThread(this);
  return 0;
}

int CMobotGroup::motionRollForwardNB(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionRollForwardThread, this);
  return 0;
}

void* CMobotGroup::motionRollForwardThread(void* arg)
{
  CMobotGroup *cmg = (CMobotGroup*)arg;
  cmg->move(cmg->argDouble, 0, 0, cmg->argDouble);
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionSkinny(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  motionSkinnyThread(this);
  return 0;
}

int CMobotGroup::motionSkinnyNB(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionSkinnyThread, this);
  return 0;
}

void* CMobotGroup::motionSkinnyThread(void* arg)
{
  CMobotGroup *cmg = (CMobotGroup*)arg;
  cmg->moveJointToNB(ROBOT_JOINT2, cmg->argDouble);
  cmg->moveJointToNB(ROBOT_JOINT3, cmg->argDouble);
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionStand()
{
  _motionInProgress++;
  motionStandThread(this);
  return 0;
}

int CMobotGroup::motionStandNB()
{
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionStandThread, NULL);
  return 0;
}

void* CMobotGroup::motionStandThread(void* arg)
{
  CMobotGroup* cmg = (CMobotGroup*)arg;
  cmg->moveToZero();
  cmg->moveJointTo(ROBOT_JOINT2, -85);
  cmg->moveJointTo(ROBOT_JOINT3, 70);
  cmg->moveWait();
  cmg->moveJointTo(ROBOT_JOINT1, 45);
  cmg->moveJointTo(ROBOT_JOINT2, 20);
  cmg->_motionInProgress--;
  return 0;
}

int CMobotGroup::motionTurnLeft(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  motionTurnLeftThread(this);
  return 0;
}

int CMobotGroup::motionTurnLeftNB(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionTurnLeftThread, this);
  return 0;
}

void* CMobotGroup::motionTurnLeftThread(void* arg)
{
  CMobotGroup* cmg = (CMobotGroup*)arg;
  cmg->move(-cmg->argDouble, 0, 0, cmg->argDouble);
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionTurnRight(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  motionTurnRightThread(this);
  return 0;
}

int CMobotGroup::motionTurnRightNB(double angle)
{
  argDouble = angle;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionTurnRightThread, this);
  return 0;
}

void* CMobotGroup::motionTurnRightThread(void* arg)
{
  CMobotGroup* cmg = (CMobotGroup*)arg;
  cmg->move(cmg->argDouble, 0, 0, -cmg->argDouble);
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionTumbleRight(int num)
{
  argInt = num;
  _motionInProgress++;
  motionTumbleRightThread(this);
  return 0;
}

int CMobotGroup::motionTumbleRightNB(int num)
{
  argInt = num;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionTumbleRightThread, this);
  return 0;
}

void* CMobotGroup::motionTumbleRightThread(void* arg)
{
  int i;
  CMobotGroup* cmg = (CMobotGroup*)arg;
  int num = cmg->argInt;

  cmg->moveToZero();

  Sleep(1000);

  for(i = 0; i < num; i++) {
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(85));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(-80));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(0));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(0));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(-80));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(-45));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(85));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(-80));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(0));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(0));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(-80));
    if(i != (num-1)) {
      cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(-45));
    }
  }
  cmg->moveJointToNB(ROBOT_JOINT3, 0);
  cmg->moveJointToNB(ROBOT_JOINT2, 0);
  cmg->moveWait();

  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionTumbleLeft(int num)
{
  argInt = num;
  _motionInProgress++;
  motionTumbleLeftThread(this);
  return 0;
}

int CMobotGroup::motionTumbleLeftNB(int num)
{
  argInt = num;
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionTumbleLeftThread, this);
  return 0;
}

void* CMobotGroup::motionTumbleLeftThread(void* arg)
{
  int i;
  CMobotGroup* cmg = (CMobotGroup*)arg;
  int num = cmg->argInt;

  cmg->moveToZero();
  Sleep(1000);

  for(i = 0; i < num; i++) {
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(-85));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(80));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(0));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(0));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(80));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(45));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(-85));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(80));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(0));
    cmg->moveJointTo(ROBOT_JOINT2, DEG2RAD(0));
    cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(80));
    if(i != (num-1)) {
      cmg->moveJointTo(ROBOT_JOINT3, DEG2RAD(45));
    }
  }
  cmg->moveJointToNB(ROBOT_JOINT2, 0);
  cmg->moveJointToNB(ROBOT_JOINT3, 0);
  cmg->moveWait();
  cmg->_motionInProgress--;
  return NULL;
}

int CMobotGroup::motionUnstand()
{
  _motionInProgress++;
  motionUnstandThread(this);
  return 0;
}

int CMobotGroup::motionUnstandNB()
{
  _motionInProgress++;
  THREAD_CREATE((pthread_t*)_thread, motionUnstandThread, NULL);
  return 0;
}

void* CMobotGroup::motionUnstandThread(void* arg)
{
  CMobotGroup* cmg = (CMobotGroup*)arg;
  cmg->moveToZero();
  cmg->moveJointTo(ROBOT_JOINT3, 45);
  cmg->moveJointTo(ROBOT_JOINT2, -85);
  cmg->moveWait();
  cmg->moveToZero();
  cmg->moveJointTo(ROBOT_JOINT2, 20);
  cmg->_motionInProgress--;
  return 0;
}

int CMobotGroup::motionWait()
{
  while(_motionInProgress > 0) {
#ifdef _WIN32
    Sleep(200);
#else
    usleep(200000);
#endif
  }
  return 0;
}

