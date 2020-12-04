foreach(LIB_DIR GLM GLFW Vulkan-Headers volk glslang VulkanMemoryAllocator)
  add_subdirectory(${LIB_DIR})
endforeach()
