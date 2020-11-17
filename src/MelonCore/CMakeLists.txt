set(TARGET_NAME MelonCore)

aux_source_directory(./ TARGET_SOURCE_FILES)

find_package(glm REQUIRED)

set(TARGET_LIBRARIES glm::glm)

setup_library()
