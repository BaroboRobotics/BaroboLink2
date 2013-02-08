cmake_minimum_required(VERSION 2.8)

project(RoboMancer)

add_subdirectory(libstkcomms)

set(GTK2_ADDITIONAL_SUFFIXES)
find_package(GTK2 2.6 REQUIRED gtk glade)

if(GTK2_FOUND)
  include_directories(${GTK2_INCLUDE_DIRS})
# For Scintilla
  include_directories(/home/dko/sys/include)
  link_directories(${LIBSTKCOMMS_BINARY_DIR})

  add_executable(RoboMancer
      commsHandler.cpp
      configFile.cpp
      connectDialog.cpp
      connectDongleDialog.cpp
      controlDialog.cpp
      programDialog.cpp
      RecordMobot.cpp
      RoboMancer.cpp
      RobotManager.cpp
      teachingDialog.cpp
      updateFirmware.cpp )
  
  target_link_libraries(RoboMancer mobot scintilla stkcomms ${GTK2_LIBRARIES} )
else(GTK2_FOUND)
  message(FATAL_ERROR "Gtk not found!")
endif(GTK2_FOUND)
