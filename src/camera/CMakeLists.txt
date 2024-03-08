cmake_minimum_required(VERSION 3.24...3.25)

set(DEPS_ROOT_DIR ${CMAKE_SOURCE_DIR}/deps)
set(W32_PTHREAD_ROOT_DIR ${DEPS_ROOT_DIR}/w32-pthreads)

# virtualcam-interface
add_library(virtualcam-interface INTERFACE)
target_sources(
  virtualcam-interface 
  INTERFACE 

  shared-memory-queue.c 
  shared-memory-queue.h 
  tiny-nv12-scale.c
  tiny-nv12-scale.h
)
target_include_directories(virtualcam-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")


# dshow 
include(cmake/libdshowcapture.cmake)
add_subdirectory(virtualcam-module)

add_library(dshow MODULE)

target_link_libraries(
  dshow
  PRIVATE 
  
  util
  strmiids
  winmm
  virtualcam-interface 
  virtualcam-guid
)

target_include_directories(
  dshow 
  PRIVATE 

  ${W32_PTHREAD_ROOT_DIR}/include 
  ${DEPS_ROOT_DIR}
)

target_sources(
  dshow 
  PRIVATE

  virtualcam.c
)

target_compile_definitions(dshow PRIVATE VIRTUALCAM_AVAILABLE UNICODE _UNICODE)

add_dependencies(dshow virtualcam-module)

set_property(TARGET dshow PROPERTY FOLDER "camera")