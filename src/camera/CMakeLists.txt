cmake_minimum_required(VERSION 3.24...3.25)

set(DEPS_ROOT_DIR ${CMAKE_SOURCE_DIR}/deps)
set(W32_PTHREAD_ROOT_DIR ${DEPS_ROOT_DIR}/w32-pthreads)

# comutils
add_library(obs-comutils INTERFACE)
add_library(COMutils ALIAS obs-comutils)
target_sources(obs-comutils INTERFACE ${DEPS_ROOT_DIR}/util/windows/ComPtr.hpp)
target_include_directories(obs-comutils INTERFACE "${DEPS_ROOT_DIR}")

# winhandle
add_library(obs-winhandle INTERFACE)
add_library(winhandle ALIAS obs-winhandle)
target_sources(obs-winhandle INTERFACE ${DEPS_ROOT_DIR}/util/windows/WinHandle.hpp)
target_include_directories(obs-winhandle INTERFACE "${DEPS_ROOT_DIR}")

# virtualcam-interface
add_library(obs-virtualcam-interface INTERFACE)
add_library(virtualcam-interface ALIAS obs-virtualcam-interface)

target_sources(
  obs-virtualcam-interface 
  INTERFACE 

  shared-memory-queue.c 
  shared-memory-queue.h 
  tiny-nv12-scale.c
  tiny-nv12-scale.h
)
target_include_directories(obs-virtualcam-interface INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")


include(cmake/libdshowcapture.cmake)
add_subdirectory(virtualcam-module)

add_library(win-dshow MODULE)
add_library(dshow ALIAS win-dshow)

target_link_libraries(
  win-dshow
  PRIVATE 
          
  strmiids
  winmm
)

# include headers
target_include_directories(
  win-dshow 
  PRIVATE 

  ${W32_PTHREAD_ROOT_DIR}/include 
  ${DEPS_ROOT_DIR}
)

target_sources(win-dshow PRIVATE virtualcam.c)
target_link_libraries(win-dshow PRIVATE virtualcam-interface virtualcam-guid)
target_compile_definitions(win-dshow PRIVATE VIRTUALCAM_AVAILABLE UNICODE _UNICODE)

add_dependencies(win-dshow obs-virtualcam-module)