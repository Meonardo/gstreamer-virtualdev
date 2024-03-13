# check_uuid: Helper function to check for valid UUID
function(check_uuid uuid_string return_value)
  set(valid_uuid TRUE)
  set(uuid_token_lengths 8 4 4 4 12)
  set(token_num 0)

  string(REPLACE "-" ";" uuid_tokens ${uuid_string})
  list(LENGTH uuid_tokens uuid_num_tokens)

  if(uuid_num_tokens EQUAL 5)
    message(DEBUG "UUID ${uuid_string} is valid with 5 tokens.")
    foreach(uuid_token IN LISTS uuid_tokens)
      list(GET uuid_token_lengths ${token_num} uuid_target_length)
      string(LENGTH "${uuid_token}" uuid_actual_length)
      if(uuid_actual_length EQUAL uuid_target_length)
        string(REGEX MATCH "[0-9a-fA-F]+" uuid_hex_match ${uuid_token})
        if(NOT uuid_hex_match STREQUAL uuid_token)
          set(valid_uuid FALSE)
          break()
        endif()
      else()
        set(valid_uuid FALSE)
        break()
      endif()
      math(EXPR token_num "${token_num}+1")
    endforeach()
  else()
    set(valid_uuid FALSE)
  endif()
  message(DEBUG "UUID ${uuid_string} valid: ${valid_uuid}")
  # cmake-format: off
  set(${return_value} ${valid_uuid} PARENT_SCOPE)
  # cmake-format: on
endfunction()

# Helper function to add a specific resource to a bundle
function(target_add_resource target resource)

  if(ARGN)
    set(target_destination "${ARGN}")
  else()
    set(target_destination "${DATA_DESTINATION}/${target}")
  endif()

  message(DEBUG "Add resource '${resource}' to target ${target} at destination '${target_destination}'...")

  install(
    FILES "${resource}"
    DESTINATION "${target_destination}"
    COMPONENT Runtime)

  add_custom_command(
    TARGET ${target}
    POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${target_destination}/"
    COMMAND "${CMAKE_COMMAND}" -E copy "${resource}" "${target_destination}/"
    COMMENT "Copy ${target} resource ${resource} to ${target_destination}"
    VERBATIM)

  source_group("Resources" FILES "${resource}")
endfunction()