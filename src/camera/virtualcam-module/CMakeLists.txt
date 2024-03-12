cmake_minimum_required(VERSION 3.24...3.25)

option(ENABLE_VIRTUALCAM "Enable Windows Virtual Camera" ON)

include(cmake/helper.cmake)

if(NOT VIRTUALCAM_GUID)
  # cmake-format: off
  set(VIRTUALCAM_GUID "" CACHE STRING "Virtual Camera GUID" FORCE)
  mark_as_advanced(VIRTUALCAM_GUID)
  # cmake-format: on

  message(WARNING "Empty Virtual Camera GUID set.")
  return()
else()
  set(VALID_GUID FALSE)
  check_uuid(${VIRTUALCAM_GUID} VALID_GUID)

  if(NOT VALID_GUID)
    message(WARNING "Invalid Virtual Camera GUID set.")
    return()
  endif()

  # DirectShow API requires separate GUID tokens
  string(REPLACE "-" ";" GUID_VALS ${VIRTUALCAM_GUID})
  list(GET GUID_VALS 0 GUID_VALS_DATA1)
  list(GET GUID_VALS 1 GUID_VALS_DATA2)
  list(GET GUID_VALS 2 GUID_VALS_DATA3)
  list(GET GUID_VALS 3 GUID_VALS_DATA4)
  list(GET GUID_VALS 4 GUID_VALS_DATA5)
  set(GUID_VAL01 ${GUID_VALS_DATA1})
  set(GUID_VAL02 ${GUID_VALS_DATA2})
  set(GUID_VAL03 ${GUID_VALS_DATA3})
  string(SUBSTRING ${GUID_VALS_DATA4} 0 2 GUID_VAL04)
  string(SUBSTRING ${GUID_VALS_DATA4} 2 2 GUID_VAL05)
  string(SUBSTRING ${GUID_VALS_DATA5} 0 2 GUID_VAL06)
  string(SUBSTRING ${GUID_VALS_DATA5} 2 2 GUID_VAL07)
  string(SUBSTRING ${GUID_VALS_DATA5} 4 2 GUID_VAL08)
  string(SUBSTRING ${GUID_VALS_DATA5} 6 2 GUID_VAL09)
  string(SUBSTRING ${GUID_VALS_DATA5} 8 2 GUID_VAL10)
  string(SUBSTRING ${GUID_VALS_DATA5} 10 2 GUID_VAL11)
endif()

add_library(virtualcam-guid INTERFACE)

configure_file(virtualcam-guid.h.in virtualcam-guid.h)
target_sources(virtualcam-guid INTERFACE "${CMAKE_CURRENT_BINARY_DIR}/virtualcam-guid.h")
target_include_directories(virtualcam-guid INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")

add_library(_virtualcam INTERFACE)

target_sources(_virtualcam INTERFACE placeholder.cpp sleepto.c sleepto.h virtualcam-filter.cpp virtualcam-filter.hpp
                                      virtualcam-module.cpp)
target_include_directories(_virtualcam INTERFACE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(_virtualcam INTERFACE VIRTUALCAM_AVAILABLE)
target_compile_options(_virtualcam INTERFACE $<IF:$<CONFIG:Debug>,/MTd,/MT>)

target_link_libraries(
  _virtualcam
  INTERFACE virtualcam-interface
            virtualcam-guid
            libdshowcapture
            winhandle
            gdiplus
            strmiids
            winmm)

add_library(virtualcam-module MODULE)

target_sources(virtualcam-module PRIVATE cmake/windows/virtualcam-module64.def)
target_link_libraries(virtualcam-module PRIVATE _virtualcam)
target_compile_definitions(virtualcam-module PRIVATE UNICODE _UNICODE)

target_include_directories(virtualcam-module PRIVATE "${DEPS_ROOT_DIR}")

set_property(TARGET virtualcam-module PROPERTY FOLDER "camera")

# POST build steps
set(OUTPUT_DATA_DEST_DIR ${DATA_DESTINATION}/camera)

configure_file(virtualcam-install.bat.in virtualcam-install.bat)
target_add_resource(virtualcam-module "${CMAKE_CURRENT_BINARY_DIR}/virtualcam-install.bat"
                    "${OUTPUT_DATA_DEST_DIR}")

configure_file(virtualcam-uninstall.bat.in virtualcam-uninstall.bat)
target_add_resource(virtualcam-module "${CMAKE_CURRENT_BINARY_DIR}/virtualcam-uninstall.bat"
                    "${OUTPUT_DATA_DEST_DIR}")

target_add_resource(virtualcam-module "${CMAKE_CURRENT_SOURCE_DIR}/data/placeholder.png"
"${OUTPUT_DATA_DEST_DIR}")

add_custom_command(
    TARGET virtualcam-module
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/virtualcam-module.dll" "${OUTPUT_DATA_DEST_DIR}"
    VERBATIM)

