cmake_minimum_required(VERSION 3.21)

project(gst-virtual-dev VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


# prohibit in-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" _LOC_PATH)
if(EXISTS "${LOC_PATH}")
	message(FATAL_ERROR "You cannot build in a source directory (or any directory with "
						"CMakeLists.txt file). Please make a build subdirectory. Feel free to "
						"remove CMakeCache.txt and CMakeFiles.")
endif()
unset(_LOC_PATH)

# setup target
add_executable(${PROJECT_NAME})

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
target_compile_definitions(${PROJECT_NAME} PRIVATE _WIN32_WINNT=0x0601 UNICODE)

target_sources(
  ${PROJECT_NAME}
  PRIVATE 
  
  src/main.cpp
)

# find GStreamer dependencies
set(ENV{PKG_CONFIG_PATH} ${GSTREAMER_PKG_DIR})

find_package(PkgConfig REQUIRED)
pkg_check_modules(
  GST REQUIRED 

  gstreamer-1.0 
  gstreamer-app-1.0 
  gstreamer-video-1.0 
  gstreamer-rtp-1.0 
  gstreamer-rtsp-1.0
)

target_include_directories(
  ${PROJECT_NAME}
  PRIVATE 

  ${GST_INCLUDE_DIRS}
)

# link GStreamer dependencies
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LIBPATH:${GST_LIBRARY_DIRS}")

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE 

  ${GST_LIBRARIES}
)