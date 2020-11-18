set(TARGET_NAME MelonCore)

aux_source_directory(./ TARGET_SOURCE_FILES)

set(TARGET_LIBRARIES glm::glm)

setup_library()
