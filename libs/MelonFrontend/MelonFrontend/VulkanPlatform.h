#pragma once

#if defined(__linux__)
#define VK_USE_PLATFORM_XLIB_KHR
#elif defined(__APPLE__)
#define VK_USE_PLATFORM_METAL_EXT
#elif defined(WIN32)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#include <volk.h>

// Vma header must be included after volk.h
#include <vk_mem_alloc.h>

#include <vector>

namespace Melon {

}  // namespace Melon
