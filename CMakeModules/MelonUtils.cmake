macro(setup_library)

  add_library(${TARGET_NAME} ${TARGET_SOURCE_FILES})

  if(TARGET_INCLUDE_DIRS)
    target_include_directories(${TARGET_NAME} PUBLIC ${TARGET_INCLUDE_DIRS})
  endif()
  if(TARGET_PRIVATE_INCLUDE_DIRS)
    target_include_directories(${TARGET_NAME}
                               PRIVATE ${TARGET_PRIVATE_INCLUDE_DIRS})
  endif()

  if(TARGET_LIBRARIES)
    target_link_libraries(${TARGET_NAME} PUBLIC ${TARGET_LIBRARIES})
  endif()
  if(TARGET_PRIVATE_LIBRARIES)
    target_link_libraries(${TARGET_NAME} PRIVATE ${TARGET_PRIVATE_LIBRARIES})
  endif()

endmacro()
