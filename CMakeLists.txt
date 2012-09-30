# === Set the project name and specify the required CMAKE version.
project(forte-corr C)
cmake_minimum_required(VERSION 2.6)

# === Compiler options
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -DDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O2 -g -DDEBUG")

# === Project version
execute_process(
  COMMAND git describe --tags
  OUTPUT_VARIABLE GIT_TAG
  ERROR_VARIABLE GIT_ERROR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(GIT_TAG)
  set(VERSION ${GIT_TAG})
else(NOT GIT_TAG)
  execute_process(
    COMMAND git describe --always
    OUTPUT_VARIABLE GIT_SHA1
    ERROR_VARIABLE GIT_ERROR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  set(VERSION ${GIT_SHA1})
endif(GIT_TAG)

configure_file(
  ${PROJECT_SOURCE_DIR}/cmake/version.h.in
  ${PROJECT_BINARY_DIR}/version.h
)

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

# === Create libraries of various management functions
#set (FORTELIB_SOURCES core/netmgmt.c core/memmgmt.c)
add_library(fortemgmt STATIC core/netmgmt.c core/memmgmt.c)
add_library(forteops STATIC core/corrops.c)
add_library(fortetest STATIC test/testops.c core/frameops.c)

# === Create various binaries.
add_executable(ioproc core/ioproc.c)
target_link_libraries (ioproc fortemgmt rt)

add_executable(cfx core/cfx.c)
target_link_libraries (cfx fortemgmt forteops rt ${FFTW_LIBRARIES} m)

add_executable(collate core/collate.c)
target_link_libraries (collate fortemgmt forteops rt)

# === Build various test related programs
add_executable(netrawsta test/netrawsta.c) 
target_link_libraries (netrawsta fortemgmt fortetest rt)

add_executable(filerawsta test/filerawsta.c) 
target_link_libraries (filerawsta fortemgmt fortetest rt m)

add_executable(xmac_set test/xmac_set.c) 
target_link_libraries (xmac_set forteops ${FFTW_LIBRARIES} m)

add_executable(fftw_shuffle test/fftw_shuffle.c)
target_link_libraries (fftw_shuffle forteops ${FFTW_LIBRARIES} m)

# === Build various tools
add_executable(frameprint tools/frameprint.c core/frameops.c) 

add_executable(shmrd tools/shmrd.c)
target_link_libraries (shmrd fortemgmt rt)

add_executable(shmfiledump tools/shmfiledump.c)
target_link_libraries (shmfiledump fortemgmt rt)
