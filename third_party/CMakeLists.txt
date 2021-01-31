set(CMAKE_CXX_STANDARD)

foreach(
  LIB_DIR
  GLM
  GLFW
  Vulkan-Headers
  volk
  glslang
  VulkanMemoryAllocator
  Assimp)
  add_subdirectory(${LIB_DIR})
endforeach()
