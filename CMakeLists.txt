cmake_minimum_required(VERSION 2.8)

project(RoboMancer)

add_subdirectory(libstkcomms)

if(CMAKE_HOST_WIN32)
  include(FindPkgConfig)
  pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
  add_definitions(-D_MSYS)
  message( "gtk cflags: ${GTK2_CFLAGS}" )
  #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GTK2_CFLAGS}")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GTK2_CFLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mms-bitfields")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mms-bitfields")
else(CMAKE_HOST_WIN32)
  set(GTK2_ADDITIONAL_SUFFIXES)
  find_package(GTK2 2.6 REQUIRED gtk glade )
endif(CMAKE_HOST_WIN32)

if(GTK2_FOUND)
  include_directories(${GTK2_INCLUDE_DIRS})
# For Scintilla
  include_directories(~/sys/include ../BaroboConfigFile)
  link_directories(${LIBSTKCOMMS_BINARY_DIR} ${GTK2_LIBRARY_DIRS} ~/sys/lib ${CMAKE_SOURCE_DIR}/../BaroboConfigFile)

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
      scanMobotsDialog.cpp
      teachingDialog.cpp
      updateFirmware.cpp )

if(CMAKE_HOST_WIN32)
  target_link_libraries(RoboMancer mobotStatic scintilla stkcomms baroboconfigfile mxml pthread ${GTK2_LIBRARIES} )
else()
  target_link_libraries(RoboMancer mobotStatic scintilla stkcomms baroboconfigfile mxml pthread bluetooth ${GTK2_LIBRARIES} )
endif()
else(GTK2_FOUND)
  message(FATAL_ERROR "Gtk not found!")
endif(GTK2_FOUND)
