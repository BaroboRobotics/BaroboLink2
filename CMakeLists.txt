cmake_minimum_required(VERSION 2.8)

project(BaroboLink)

add_subdirectory(libstkcomms)
add_subdirectory(libbarobo)

if(CMAKE_HOST_WIN32)
  include(FindPkgConfig)
  pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
  #pkg_check_modules(PYTHON REQUIRED python)
  find_package(PythonLibs 2.7 REQUIRED python)
  add_definitions(-D_MSYS -DMSYS)
  message( "gtk cflags: ${GTK2_CFLAGS}" )
  #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GTK2_CFLAGS}")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GTK2_CFLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mms-bitfields")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mms-bitfields")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -mms-bitfields -mwindows")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -mms-bitfields -mwindows")
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -g -mms-bitfields")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -mms-bitfields")
else(CMAKE_HOST_WIN32)
  set(GTK2_ADDITIONAL_SUFFIXES)
  find_package(GTK2 2.6 REQUIRED gtk glade )
  find_package(PythonLibs 2.7 REQUIRED python)
endif(CMAKE_HOST_WIN32)

if(GTK2_FOUND)
  include_directories(${GTK2_INCLUDE_DIRS})
  include_directories(${PYTHON_INCLUDE_DIRS})
# For Scintilla
  include_directories(${CMAKE_SOURCE_DIR}/libbarobo)
  include_directories(${CMAKE_SOURCE_DIR}/libbarobo/BaroboConfigFile)
  include_directories(${CMAKE_SOURCE_DIR}/libbarobo/BaroboConfigFile/mxml-2.7)
  include_directories(${CMAKE_SOURCE_DIR}/scintilla/include)
  include_directories(${LIBSTKCOMMS_SOURCE_DIR})
  link_directories(${LIBSTKCOMMS_BINARY_DIR})
  link_directories(${GTK2_LIBRARY_DIRS})
  link_directories(${PYTHON_LIBRARY_DIRS})
  link_directories(${CMAKE_SOURCE_DIR}/libbarobo)
  link_directories(${CMAKE_SOURCE_DIR}/libbarobo/BaroboConfigFile)

  add_executable(BaroboLink
      commsHandler.cpp
      configFile.cpp
      connectDialog.cpp
      connectDongleDialog.cpp
      controlDialog.cpp
      programDialog.cpp
      RecordMobot.cpp
      BaroboLink.cpp
      RobotManager.cpp
      scanMobotsDialog.cpp
      teachingDialog.cpp
      updateFirmware.cpp )

  add_executable(MobotFirmwareUpdate MobotFirmwareUpdate.cpp)

if(CMAKE_HOST_WIN32)
  target_link_libraries(
      BaroboLink
      ${CMAKE_SOURCE_DIR}/scintilla/bin/scintilla.a 
      stkcomms 
      mobotStatic
      ${GTK2_LIBRARIES} 
      ${PYTHON_LIBRARIES})
  target_link_libraries(
      MobotFirmwareUpdate
      stkcomms 
      mobotStatic
      ${GTK2_LIBRARIES} )
else()
  target_link_libraries(BaroboLink mobotStatic ${CMAKE_SOURCE_DIR}/scintilla/bin/scintilla.a stkcomms pthread bluetooth ${GTK2_LIBRARIES} ${PYTHON_LIBRARIES} )
  target_link_libraries(MobotFirmwareUpdate mobotStatic stkcomms pthread ${GTK2_LIBRARIES} )
endif()
else(GTK2_FOUND)
  message(FATAL_ERROR "Gtk not found!")
endif(GTK2_FOUND)
