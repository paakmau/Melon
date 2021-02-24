#pragma once

#include <MelonFrontend/VulkanPlatform.h>

namespace Melon {

struct Texture {
    VkImage image;
    VkImageView imageView;
    VmaAllocation allocation;
    VkExtent2D extent;
    VkFormat format;
};

}  // namespace Melon
