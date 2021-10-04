set(TARGET_NAME MelonTask)

file(GLOB TARGET_HEADER_FILES ${TARGET_NAME}/*.h)
aux_source_directory(${TARGET_NAME} TARGET_SOURCE_FILES)

add_library(${TARGET_NAME} ${TARGET_SOURCE_FILES} ${TARGET_HEADER_FILES})

target_include_directories(${TARGET_NAME} PUBLIC .)
target_link_libraries(${TARGET_NAME} PRIVATE Threads::Threads)
