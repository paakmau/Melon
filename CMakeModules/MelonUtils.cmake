macro(setup_library)

  file(GLOB TARGET_HEADER_FILES $<BUILD_INTERFACE:"${PROJECT_SOURCE_DIR}/${TARGET_NAME}/*.h">)

  add_library(${TARGET_NAME} ${TARGET_SOURCE_FILES} ${TARGET_HERDER_FILES})

  target_include_directories(
    ${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

  if(TARGET_INCLUDE_DIRS)
    target_include_directories(${TARGET_NAME} PUBLIC ${TARGET_INCLUDE_DIRS})
  endif()
  if(TARGET_PRIVATE_INCLUDE_DIRS)
    target_include_directories(MelonGraphics
                               PRIVATE ${TARGET_PRIVATE_INCLUDE_DIRS})
  endif()

  if(TARGET_LIBRARIES)
    target_link_libraries(${TARGET_NAME} PUBLIC ${TARGET_LIBRARIES})
  endif()
  if(TARGET_PRIVATE_LIBRARIES)
    target_link_libraries(${TARGET_NAME} PRIVATE ${TARGET_PRIVATE_LIBRARIES})
  endif()

endmacro()
