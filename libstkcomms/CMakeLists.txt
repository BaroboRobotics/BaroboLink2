# The ODE Library

cmake_minimum_required(VERSION 2.6)
#include_directories(../opende/sys/include)

project(LIBSTKCOMMS)


if(CMAKE_HOST_WIN32)
	add_library(stkcomms STATIC
	  libstkcomms.cpp
	  )
	add_definitions(-DBUILD_CSTKCOMMS)
	target_link_libraries(stkcomms Ws2_32)
  set_target_properties(stkcomms PROPERTIES COMPILE_FLAGS "/GS-")
elseif(CMAKE_HOST_APPLE)
  set(CMAKE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib")
	add_definitions(-DBUILD_CSTKCOMMS)
  add_library(stkcomms STATIC
	  libstkcomms.cpp
	  libstkcomms++.cpp
	  )
	target_link_libraries(stkcomms pthread)
  set_target_properties(stkcomms PROPERTIES
    INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
  install(TARGETS stkcomms 
          LIBRARY DESTINATION lib)
  install(FILES libstkcomms.hpp DESTINATION include)
else(CMAKE_HOST_WIN32)
	add_definitions(-DBUILD_CSTKCOMMS)
  add_library(stkcomms STATIC
	  libstkcomms.cpp
	  libstkcomms++.cpp
	  )
  #target_link_libraries(stkcomms bluetooth pthread rt)
  install(TARGETS stkcomms 
          LIBRARY DESTINATION lib
          ARCHIVE DESTINATION lib)
  install(FILES libstkcomms.h DESTINATION include)
endif(CMAKE_HOST_WIN32)

