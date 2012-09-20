AUTOMAKE_OPTIONS = foreign

AM_CPPFLAGS=$(DEPS_CFLAGS)
#LDFLAGS=$(DEPS_LFLAGS)

bin_PROGRAMS = RoboMancer
RoboMancer_SOURCES = commsHandler.cpp  configFile.cpp  connectDialog.cpp  controlDialog.cpp  programDialog.cpp  RecordMobot.cpp  RoboMancer.cpp  RobotManager.cpp  teachingDialog.cpp updateFirmware.cpp 
RoboMancer_LDADD = $(DEPS_LIBS) 
pkgdata_DATA = 16px_move_back.png  16px_move_forward.png  16px_stop.png  imobot_diagram.png  move_back.png  move_forward.png  rotate_left.png  rotate_right.png  stop.png interface/interface.glade interface/rev3.hex interface/rev4.hex interface/rev3_safe.hex
