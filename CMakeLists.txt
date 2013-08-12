cmake_minimum_required(VERSION 2.8)

project(BaroboLink)

add_subdirectory(libstkcomms)
add_subdirectory(libbarobo)
exec_program(cat ARGS VERSION.TXT OUTPUT_VARIABLE barobolink_version)
add_definitions(-DBAROBOLINK_VERSION=${barobolink_version})

if(CMAKE_HOST_WIN32)
  include(FindPkgConfig)
  pkg_check_modules(GTK2 REQUIRED gtk+-2.0)
  #pkg_check_modules(PYTHON REQUIRED python)
#find_package(PythonLibs 2.7 REQUIRED python)
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
#find_package(PythonLibs 2.7 REQUIRED python)
endif(CMAKE_HOST_WIN32)

if(GTK2_FOUND)
  include_directories(${GTK2_INCLUDE_DIRS})
  include_directories(${PYTHON_INCLUDE_DIRS})
# For Scintilla
  include_directories(${CMAKE_SOURCE_DIR}/libbarobo)
  include_directories(${CMAKE_SOURCE_DIR}/libbarobo/BaroboConfigFile)
  include_directories(${CMAKE_SOURCE_DIR}/libbarobo/BaroboConfigFile/mxml-2.7)
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
      RecordMobot.cpp
      BaroboLink.cpp
      RobotManager.cpp
      scanMobotsDialog.cpp
      teachingDialog.cpp
      updateFirmware.cpp )

  add_executable(BaroboFirmwareUpdate BaroboFirmwareUpdate.cpp)

if(CMAKE_HOST_WIN32)
  target_link_libraries(
      BaroboLink
      stkcomms 
      baroboStatic
      ${CMAKE_SOURCE_DIR}/BaroboLink.res
      ${GTK2_LIBRARIES} 
      ${PYTHON_LIBRARIES})
  target_link_libraries(
      BaroboFirmwareUpdate
      stkcomms 
      baroboStatic
      ${GTK2_LIBRARIES} )
else()
  target_link_libraries(BaroboLink baroboStatic stkcomms pthread bluetooth ${GTK2_LIBRARIES} ${PYTHON_LIBRARIES} )
  target_link_libraries(BaroboFirmwareUpdate baroboStatic stkcomms pthread ${GTK2_LIBRARIES} )
  install(TARGETS BaroboLink BaroboFirmwareUpdate
          RUNTIME DESTINATION bin )
  install(FILES 
      interface/16px_move_back.png
      interface/16px_move_back.svg
      interface/16px_move_forward.png
      interface/16px_move_forward.svg
      interface/16px_stop.png
      interface/16px_stop.svg
      interface/DOF_joint_diagram.png
      interface/face_backward.svg
      interface/imobot_diagram.png
      interface/imobot_diagram.svg
      interface/iMobot.png
      interface/inch_left.svg
      interface/inch_right.svg
      interface/interface.glade
      interface/interfacepatch.patch
      interface/lface_forward.svg
      interface/mobotfirmwareupdateinterface.glade
      interface/move_back.png
      interface/move_back.svg
      interface/move_forward.png
      interface/move_forward.svg
      interface/rotate_left.png
      interface/rotate_left.svg
      interface/rotate_right.png
      interface/rotate_right.svg
      interface/stop.png
      interface/stop.svg
      DESTINATION share/BaroboLink)
  install(FILES
      BaroboLink.desktop 
      DESTINATION share/applications
      )
endif()
else(GTK2_FOUND)
  message(FATAL_ERROR "Gtk not found!")
endif(GTK2_FOUND)

if(CMAKE_SYSTEM_NAME MATCHES [Ll]inux)
  string(TOLOWER "${CPACK_PACKAGE_NAME}" CPACK_PACKAGE_NAME_LOWERCASE)
  find_program(DPKG_PROGRAM dpkg DOC "dpkg program of Debian-based systems")
  if(DPKG_PROGRAM)
       execute_process(COMMAND ${DPKG_PROGRAM} --print-architecture
                 OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
                 OUTPUT_STRIP_TRAILING_WHITESPACE)
       set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${PROJECT_VERSION}-${CPACK_DEBIAN_PACKAGE_REVISION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}") 
  else(DPKG_PROGRAM)
       set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME_LOWERCASE}_${PROJECT_VERSION}-${CPACK_DEBIAN_PACKAGE_REVISION}_${CMAKE_SYSTEM_NAME}")
  endif(DPKG_PROGRAM)

  INCLUDE (InstallRequiredSystemLibraries)

  SET (CPACK_SET_DESTDIR "on")
  SET (CPACK_PACKAGE_NAME "BaroboLink")
  SET (CPACK_PACKAGING_INSTALL_PREFIX "/tmp")
  SET (CPACK_GENERATOR "DEB")

  SET (VERSION 1.2.0)
  SET (CPACK_DEBIAN_PACKAGE_VERSION ${VERSION})
  SET (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
  SET (CPACK_DEBIAN_PACKAGE_SECTION "Science & Engineering")
  SET (CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
#SET (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libgladeui-1-11, libglib2.0-0, libgtk2.0-0")
  set (CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

  SET (CPACK_PACKAGE_DESCRIPTION "Barobo Robot Controller")
  SET (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Graphical Interface for controlling, programming, and interacting with Barobo Mobots and Linkbots.")
  SET (CPACK_PACKAGE_CONTACT "david@barobo.com")
  SET (CPACK_DEBIAN_PACKAGE_MAINTAINER "David Ko <david@barobo.com>")
  SET (CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${VERSION}_${CPACK_DEBIAN_PACKAGE_ARCHITECTURE}")

  SET (CPACK_COMPONENTS_ALL Libraries ApplicationData)
  INCLUDE (CPack)
endif()
