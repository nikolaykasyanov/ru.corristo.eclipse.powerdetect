cmake_minimum_required(VERSION 2.8)

project (libpowersrc)

find_package(JNI REQUIRED)
find_library(CORE_FOUNDATION CoreFoundation REQUIRED)
find_library(IOKIT IOKit REQUIRED)

include_directories(${JNI_INCLUDE_DIRS} ${CORE_FOUNDATION_INCLUDE_DIRS} ${IOKIT_INCLUDE_DIRS})

set(LIBS ${LIBS} ${CORE_FOUNDATION} ${IOKIT} ${JNI_LIBRARIES})

add_library(powersrc SHARED powersrc.c)

target_link_libraries(powersrc ${LIBS})

set_target_properties(powersrc PROPERTIES SUFFIX ".jnilib")