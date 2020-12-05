set(TARGET_NAME MelonCore)

aux_source_directory(./src TARGET_SOURCE_FILES)

set(TARGET_INCLUDE_DIRS ./include)

set(TARGET_LIBRARIES MelonTask glm::glm)

setup_library()
