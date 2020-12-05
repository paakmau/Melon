set(TARGET_NAME MelonFrontend)

aux_source_directory(./src TARGET_SOURCE_FILES)

set(TARGET_INCLUDE_DIRS ./include)

set(TARGET_PRIVATE_LIBRARIES glfw)
set(TARGET_LIBRARIES MelonCore Vulkan::Headers volk SPIRV vkmemalloc)

setup_library()
