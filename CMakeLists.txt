# === Set the project name and specify the required CMAKE version.
project(forte C)
cmake_minimum_required(VERSION 2.6)

# === Compiler options
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")

#if(GIT_TAG)
#  set(VERSION ${GIT_TAG})
#else(NOT GIT_TAG)
#  execute_process(
#    COMMAND git describe --always
#    OUTPUT_VARIABLE GIT_SHA1
#    ERROR_VARIABLE GIT_ERROR
#    OUTPUT_STRIP_TRAILING_WHITESPACE
#  )
#  set(VERSION ${GIT_SHA1})
#endif(GIT_TAG)
#
#configure_file(
#  ${PROJECT_SOURCE_DIR}/cmake/version.h.in
#  ${PROJECT_BINARY_DIR}/version.h
#)

# === Set compiled output binary path
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# === Set cmake 3rd library modules path
SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/modules")

# === Find Dependencies.
find_package(FFTW REQUIRED)
include_directories(${FFTW_INCLUDE_DIR})

# === Include sources.
include(cmake/sources.cmake)
include_directories(${PROJECT_BINARY_DIR})

# === Create various binaries.
add_executable(ioproc ${IOPROC_SOURCES})
target_link_libraries (ioproc rt)

add_executable(cfx ${CFX_SOURCES})
target_link_libraries (cfx rt ${FFTW_LIBRARIES} m)

# === Build various test related programs
add_executable(netrawsta test/netrawsta.c core/memmgmt.c core/netmgmt.c) 
target_link_libraries (netrawsta rt)

add_executable(filerawsta test/filerawsta.c test/testops.c core/memmgmt.c core/netmgmt.c) 
target_link_libraries (filerawsta rt m)

add_executable(xmac_set test/xmac_set.c core/corrops.c) 
target_link_libraries (xmac_set ${FFTW_LIBRARIES} m)

add_executable(fftw_shuffle test/fftw_shuffle.c core/corrops.c) 
target_link_libraries (fftw_shuffle ${FFTW_LIBRARIES} m)

# === Build various tools
add_executable(frameprint tools/frameprint.c core/frameops.c) 

add_executable(shmrd tools/shmrd.c core/memmgmt.c core/netmgmt.c) 
target_link_libraries (shmrd rt)

add_executable(shmfiledump tools/shmfiledump.c core/memmgmt.c core/netmgmt.c) 
target_link_libraries (shmfiledump rt)

