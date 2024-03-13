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


# camera 
include(cmake/libdshowcapture.cmake)
add_subdirectory(virtualcam-module)

add_library(
  camera 
  SHARED 

  virtualcam.h
  virtualcam.c
)

target_compile_definitions(camera PRIVATE VIRTUALCAM_AVAILABLE UNICODE _UNICODE)

target_link_libraries(
  camera
  PRIVATE 
  
  util
  strmiids
  winmm
  virtualcam-interface 
  virtualcam-guid
)

target_include_directories(
  camera 
  PRIVATE 

  ${W32_PTHREAD_ROOT_DIR}/include 
  ${DEPS_ROOT_DIR}
)

# add virtualcam-module as a dependency
add_dependencies(camera virtualcam-module)

# copy camera.dll to build directory
add_custom_command(
    TARGET camera
    POST_BUILD

    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/camera.dll" "${CMAKE_BINARY_DIR}/$<CONFIG>"
)

set_property(TARGET camera PROPERTY FOLDER "camera")