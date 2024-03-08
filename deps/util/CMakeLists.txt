cmake_minimum_required(VERSION 3.22...3.25)

set(CMAKE_CXX_STANDARD 17)

set(DEPS_ROOT_DIR ${CMAKE_SOURCE_DIR}/deps)
set(W32_PTHREAD_ROOT_DIR ${DEPS_ROOT_DIR}/w32-pthreads)

# comutils
add_library(comutils INTERFACE)
target_sources(comutils INTERFACE ${DEPS_ROOT_DIR}/util/windows/ComPtr.hpp)
target_include_directories(comutils INTERFACE "${DEPS_ROOT_DIR}")

# winhandle
add_library(winhandle INTERFACE)
target_sources(winhandle INTERFACE ${DEPS_ROOT_DIR}/util/windows/WinHandle.hpp)
target_include_directories(winhandle INTERFACE "${DEPS_ROOT_DIR}")

# util library
add_library(util SHARED)

target_sources(
    util
    PRIVATE 

    ${CMAKE_CURRENT_SOURCE_DIR}/base.c
    ${CMAKE_CURRENT_SOURCE_DIR}/base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/bmem.c
    ${CMAKE_CURRENT_SOURCE_DIR}/bmem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/c99defs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/circlebuf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/darray.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dstr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/dstr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dstr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platform.c
    ${CMAKE_CURRENT_SOURCE_DIR}/platform.h
    ${CMAKE_CURRENT_SOURCE_DIR}/threading.h
    ${CMAKE_CURRENT_SOURCE_DIR}/utf8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/utf8.h
    ${CMAKE_CURRENT_SOURCE_DIR}/platform-windows.c
    ${CMAKE_CURRENT_SOURCE_DIR}/threading-windows.c
    ${CMAKE_CURRENT_SOURCE_DIR}/threading-windows.h
    ${CMAKE_CURRENT_SOURCE_DIR}/util_uint64.h

    ${CMAKE_CURRENT_SOURCE_DIR}/windows/CoTaskMemPtr.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/windows/HRError.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/windows/win-registry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/windows/win-version.h
    ${CMAKE_CURRENT_SOURCE_DIR}/windows/window-helpers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/windows/window-helpers.h
)

target_link_libraries(
    util
    PRIVATE 
  
    Avrt
    Dwmapi
    Dxgi
    winmm
    Rpcrt4

    winhandle
    comutils

    ${W32_PTHREAD_ROOT_DIR}/lib/$<CONFIG>/w32-pthreads.lib 
)

target_include_directories(
    util
    PRIVATE

    ${W32_PTHREAD_ROOT_DIR}/include
)