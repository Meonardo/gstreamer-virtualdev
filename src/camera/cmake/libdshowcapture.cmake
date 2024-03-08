add_library(libdshowcapture INTERFACE)

target_sources(
  libdshowcapture
  INTERFACE ${DEPS_ROOT_DIR}/libdshowcapture/dshowcapture.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/capture-filter.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/capture-filter.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/device-vendor.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/device.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/device.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-base.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-base.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-demux.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-demux.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-device-defs.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-encoded-device.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-enum.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-enum.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-formats.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-formats.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-media-type.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshow-media-type.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshowcapture.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/dshowencode.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/encoder.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/encoder.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/external/IVideoCaptureFilter.h
            ${DEPS_ROOT_DIR}/libdshowcapture/source/log.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/log.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/output-filter.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/source/output-filter.hpp
            ${DEPS_ROOT_DIR}/libdshowcapture/external/capture-device-support/Library/EGAVResult.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/external/capture-device-support/Library/ElgatoUVCDevice.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/external/capture-device-support/Library/win/EGAVHIDImplementation.cpp
            ${DEPS_ROOT_DIR}/libdshowcapture/external/capture-device-support/SampleCode/DriverInterface.cpp)

target_include_directories(
  libdshowcapture INTERFACE "${DEPS_ROOT_DIR}/libdshowcapture"
                            "${DEPS_ROOT_DIR}/libdshowcapture/external/capture-device-support/Library")

target_compile_definitions(libdshowcapture INTERFACE _UP_WINDOWS=1)
target_compile_options(libdshowcapture INTERFACE /wd4018)

get_target_property(target_sources libdshowcapture INTERFACE_SOURCES)
set(target_headers ${target_sources})
set(target_external_sources ${target_sources})

list(FILTER target_external_sources INCLUDE REGEX ".+external/.+/.+\\.cpp")
list(FILTER target_sources EXCLUDE REGEX ".+external/.+/.+\\.cpp")
list(FILTER target_sources INCLUDE REGEX ".*\\.(m|c[cp]?p?|swift)")
list(FILTER target_headers INCLUDE REGEX ".*\\.h(pp)?")

source_group("libdshowcapture-external\\Source Files" FILES ${target_external_sources})
source_group("libdshowcapture\\Source Files" FILES ${target_sources})
source_group("libdshowcapture\\Header Files" FILES ${target_headers})
