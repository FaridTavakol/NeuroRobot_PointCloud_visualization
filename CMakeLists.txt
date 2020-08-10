PROJECT(Point_receiver)

cmake_minimum_required(VERSION 3.0.0)
if(COMMAND cmake_policy)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

find_package(OpenIGTLink REQUIRED)

include(${OpenIGTLink_USE_FILE})

ADD_EXECUTABLE(point_server   point_server.cpp)
TARGET_LINK_LIBRARIES(point_server OpenIGTLink)

ADD_EXECUTABLE(point_client   point_client.cpp)
TARGET_LINK_LIBRARIES(point_client OpenIGTLink)