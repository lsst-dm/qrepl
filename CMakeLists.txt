cmake_minimum_required(VERSION 3.2)
project(qrepl)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif(NOT CMAKE_BUILD_TYPE)

enable_testing()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic -Wno-variadic-macros -fmessage-length=0")

include_directories(
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_BINARY_DIR}"
    "$ENV{BOOST_DIR}/include"
    "$ENV{CAPNPROTO_DIR}/include"
    "$ENV{LOG_DIR}/include"
    "$ENV{LOG4CXX_DIR}/include"
)

link_directories(
    "$ENV{BOOST_DIR}/lib"
    "$ENV{CAPNPROTO_DIR}/lib"
    "$ENV{LOG_DIR}/lib"
    "$ENV{LOG4CXX_DIR}/lib"
)

add_definitions(-DKJ_STD_COMPAT)

add_subdirectory(qhttp)
add_subdirectory(qdht)
add_subdirectory(qreplnode)
add_subdirectory(qreplmon)
