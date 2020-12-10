set(TARGET_NAME MelonTask)

aux_source_directory(./src TARGET_SOURCE_FILES)

set(TARGET_INCLUDE_DIRS ./include)

set(TARGET_LIBRARIES Threads::Threads)

setup_library()
