add_custom_command(
  TARGET virtualcam-module
  POST_BUILD
  COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_SOURCE_DIR}/build_x86 --config $<CONFIG> -t virtualcam-module
  COMMENT "Build 32-bit virtualcam")

# copy 32bit virtual camera to output directory
add_custom_command(
  TARGET 
  virtualcam-module

  POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E copy "${CMAKE_SOURCE_DIR}/build_x86/src/camera/virtualcam-module/$<CONFIG>/${OUTPUT_NAME}32.dll" "${OUTPUT_DATA_DEST_DIR}"
  VERBATIM
)