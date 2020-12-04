#pragma once

#include <MelonFrontend/SpirvUtil.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <array>
#include <cstddef>
#include <fstream>
#include <set>
#include <vector>

namespace MelonFrontend {

static VkShaderModule createShaderModule(VkDevice device, const std::vector<unsigned int>& spirv) {
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = spirv.size() * sizeof(uint32_t),
        .pCode = static_cast<const uint32_t*>(spirv.data())};
    VkShaderModule shaderModule;
    VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
    assert(result == VK_SUCCESS);
    return shaderModule;
}

inline void createInstance(const std::vector<const char*>& requiredVulkanInstanceExtensions, VkInstance& instance) {
    // TODO: Define the application name and engine name

    // TODO: Enable validation layers for debug
    // TODO: TEMP
    std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};

    // Application info
    VkApplicationInfo applicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .apiVersion = VK_API_VERSION_1_0};
    // Instance create info
    VkInstanceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &applicationInfo,
        .enabledExtensionCount = static_cast<uint32_t>(requiredVulkanInstanceExtensions.size()),
        .ppEnabledExtensionNames = requiredVulkanInstanceExtensions.data()};
    // Create instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    assert(result == VK_SUCCESS);
}

inline void selectPhysicalDevice(VkInstance instance, VkSurfaceKHR surface, VkPhysicalDevice& physicalDevice, uint32_t& graphicsQueueFamilyIndex, uint32_t& presentQueueFamilyIndex, VkPhysicalDeviceFeatures& features) {
    uint32_t physicalDeviceCount;
    VkResult result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    assert(result == VK_SUCCESS && physicalDeviceCount > 0);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    result = vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    assert(result == VK_SUCCESS);
    physicalDevice = VK_NULL_HANDLE;

    for (uint32_t i = 0; i < physicalDeviceCount; ++i) {
        physicalDevice = physicalDevices[i];

        // TODO: Check if the device support our Vulkan version
        VkPhysicalDeviceProperties physicalDeviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

        // Check if the device has a graphics queue and a present queue
        uint32_t queueFamiliesCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, nullptr);
        if (queueFamiliesCount == 0)
            continue;
        std::vector<VkQueueFamilyProperties> queueFamiliesProperties(queueFamiliesCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamiliesCount, queueFamiliesProperties.data());
        bool hasGraphicsQueueFamily = false;
        bool hasPresentQueueFamily = false;
        for (uint32_t j = 0; j < queueFamiliesCount; ++j) {
            VkQueueFamilyProperties props = queueFamiliesProperties[j];
            if (props.queueCount == 0)
                continue;
            if (props.queueFlags & VK_QUEUE_GRAPHICS_BIT)
                graphicsQueueFamilyIndex = j, hasGraphicsQueueFamily = true;

            VkBool32 presentSupport;
            result = vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            assert(result == VK_SUCCESS);
            if (presentSupport)
                presentQueueFamilyIndex = j, hasPresentQueueFamily = true;

            if (hasGraphicsQueueFamily && hasPresentQueueFamily) break;
        }
        if (!hasGraphicsQueueFamily || !hasPresentQueueFamily) continue;

        // Check if the device support VK_KHR_swapchain extension
        uint32_t extensionCount;
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
        assert(result == VK_SUCCESS);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        result = vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
        assert(result == VK_SUCCESS);
        bool supportsSwapchain = false;
        for (uint32_t k = 0; k < extensionCount; ++k)
            if (!strcmp(extensions[k].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME))
                supportsSwapchain = true;
        if (!supportsSwapchain) continue;

        // Proper physical device found
        vkGetPhysicalDeviceFeatures(physicalDevice, &features);

        break;
    }
}

inline void createLogicalDevice(VkPhysicalDevice physicalDevice, const uint32_t& graphicsQueueFamilyIndex, const uint32_t& presentQueueFamilyIndex, VkPhysicalDeviceFeatures physicalDeviceFeatures, VkDevice& device, VkQueue& graphicsQueue, VkQueue& presentQueue) {
    // Device queue create info
    const float queuePriority = 1.0f;
    std::vector<const char*> deviceExtensionNames = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    std::set<uint32_t> uniqueQueueFamilyIndicies{graphicsQueueFamilyIndex, presentQueueFamilyIndex};
    std::vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    for (uint32_t queueFamilyIndex : uniqueQueueFamilyIndicies)
        deviceQueueCreateInfos.emplace_back(
            VkDeviceQueueCreateInfo{.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                                    .queueFamilyIndex = queueFamilyIndex,
                                    .queueCount = 1,
                                    .pQueuePriorities = &queuePriority});

    // Device create info
    // TODO: Don't enable features unnecessary
    const VkPhysicalDeviceFeatures& enabledFeatures = physicalDeviceFeatures;
    VkDeviceCreateInfo deviceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = static_cast<uint32_t>(deviceQueueCreateInfos.size()),
        .pQueueCreateInfos = deviceQueueCreateInfos.data(),
        .pEnabledFeatures = &enabledFeatures,
        .enabledExtensionCount = (uint32_t)deviceExtensionNames.size(),
        .ppEnabledExtensionNames = deviceExtensionNames.data()};
    VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &device);
    assert(result == VK_SUCCESS);

    // Retrieve queue handles
    vkGetDeviceQueue(device, graphicsQueueFamilyIndex, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueFamilyIndex, 0, &presentQueue);
}

inline void createSwapchain(VkDevice device, VkExtent2D windowExtent, VkSurfaceKHR surface, VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex, uint32_t presentQueueFamilyIndex, VkSwapchainKHR& swapchain, VkFormat& imageFormat, VkExtent2D& imageExtent, std::vector<VkImage>& images, std::vector<VkImageView>& imageViews) {
    // Get Swapchain support details
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);
    assert(result == VK_SUCCESS);
    uint32_t availableFormatCount;
    result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableFormatCount, nullptr);
    assert(result == VK_SUCCESS);
    std::vector<VkSurfaceFormatKHR> availableFormats(availableFormatCount);
    if (availableFormatCount != 0) {
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &availableFormatCount, availableFormats.data());
        assert(result == VK_SUCCESS);
    }
    uint32_t availablePresentModeCount;
    result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &availablePresentModeCount, nullptr);
    assert(result == VK_SUCCESS);
    std::vector<VkPresentModeKHR> availablePresentModes(availablePresentModeCount);
    if (availablePresentModeCount != 0) {
        result = vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &availablePresentModeCount, availablePresentModes.data());
        assert(result == VK_SUCCESS);
    }

    // Choose a proper surface format
    VkSurfaceFormatKHR surfaceFormat;
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        surfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    else {
        surfaceFormat = availableFormats[0];
        for (const auto& availableFormat : availableFormats)
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                surfaceFormat = availableFormat;
    }

    // Choose a proper present mode
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
    for (const auto& availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = availablePresentMode;
            break;
        } else if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR)
            presentMode = availablePresentMode;

    // Choose the extent
    VkExtent2D extent;
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        extent = surfaceCapabilities.currentExtent;
    else {
        extent = windowExtent;
        extent.width = std::max(surfaceCapabilities.minImageExtent.width, std::min(surfaceCapabilities.maxImageExtent.width, extent.width));
        extent.height = std::max(surfaceCapabilities.minImageExtent.height, std::min(surfaceCapabilities.maxImageExtent.height, extent.height));
    }

    // Choose the image count
    uint32_t imageCount = surfaceCapabilities.minImageCount + 1;
    if (surfaceCapabilities.maxImageCount > 0 && imageCount > surfaceCapabilities.maxImageCount)
        imageCount = surfaceCapabilities.maxImageCount;

    const auto compositeAlpha = (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // Create Swapchain
    VkSwapchainCreateInfoKHR swapchainCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = compositeAlpha,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE};
    uint32_t queueFamilyIndices[] = {graphicsQueueFamilyIndex, presentQueueFamilyIndex};
    if (graphicsQueueFamilyIndex == presentQueueFamilyIndex)
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    else {
        swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    result = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &swapchain);
    assert(result == VK_SUCCESS);

    // Store the format and extent chosen for the swapchain
    imageFormat = surfaceFormat.format;
    imageExtent = extent;

    // Retrive swapchain images, and then create swap image contexts
    result = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    assert(result == VK_SUCCESS);
    images.resize(imageCount);
    result = vkGetSwapchainImagesKHR(device, swapchain, &imageCount, images.data());
    assert(result == VK_SUCCESS);
    imageViews.resize(imageCount);
    for (uint32_t i = 0; i < images.size(); i++) {
        VkImageViewCreateInfo imageViewCreateInfo{
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = imageFormat,
            .components.r = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.b = VK_COMPONENT_SWIZZLE_IDENTITY,
            .components.a = VK_COMPONENT_SWIZZLE_IDENTITY,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.baseMipLevel = 0,
            .subresourceRange.levelCount = 1,
            .subresourceRange.baseArrayLayer = 0,
            .subresourceRange.layerCount = 1};
        result = vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageViews[i]);
        assert(result == VK_SUCCESS);
    }
}

inline void createAllocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, VmaAllocator& allocator) {
    const VmaVulkanFunctions vulkanFunctions{
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR,
        .vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR};
    const VmaAllocatorCreateInfo createInfo{
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &vulkanFunctions,
        .instance = instance};
    VkResult result = vmaCreateAllocator(&createInfo, &allocator);
    assert(result == VK_SUCCESS);
}

inline void createCommandPool(VkDevice device, VkCommandPoolCreateFlags flags, uint32_t graphicsQueueFamilyIndex, VkCommandPool& commandPool) {
    VkCommandPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = flags,
        .queueFamilyIndex = graphicsQueueFamilyIndex};
    VkResult result = vkCreateCommandPool(device, &createInfo, nullptr, &commandPool);
    assert(result == VK_SUCCESS);
}

inline void allocateCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t count, VkCommandBuffer& commandBuffers) {
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = commandPool,
        .level = level,
        .commandBufferCount = count};
    VkResult result = vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffers);
    assert(result == VK_SUCCESS);
}

inline void createRenderPass(VkDevice device, VkFormat imageFormat, VkRenderPass& renderPass) {
    VkAttachmentDescription colorAttachmentDescription{
        .format = imageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR};
    std::array<VkAttachmentDescription, 1> attachments = {colorAttachmentDescription};
    VkAttachmentReference colorAttachmentReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

    VkSubpassDescription subpassDescription{
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference};

    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT};

    VkRenderPassCreateInfo renderPassCreateInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = static_cast<uint32_t>(attachments.size()),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpassDescription,
        .dependencyCount = 1,
        .pDependencies = &dependency};

    VkResult result = vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass);
    assert(result == VK_SUCCESS);
}

inline void createFramebuffer(VkDevice device, VkImageView imageViews, VkRenderPass renderPass, VkExtent2D extent, VkFramebuffer& framebuffer) {
    VkFramebufferCreateInfo framebufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = renderPass,
        .attachmentCount = 1,
        .pAttachments = &imageViews,
        .width = extent.width,
        .height = extent.height,
        .layers = 1};
    VkResult result = vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &framebuffer);
    assert(result == VK_SUCCESS);
}

template <size_t Count>
inline void createSemaphores(VkDevice device, const unsigned int& count, std::array<VkSemaphore, Count>& semaphores) {
    VkSemaphoreCreateInfo createInfo{.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    for (unsigned int i = 0; i < count; i++) {
        VkResult result = vkCreateSemaphore(device, &createInfo, nullptr, &semaphores[i]);
        assert(result == VK_SUCCESS);
    }
}

template <size_t Count>
inline void createFences(VkDevice device, VkFenceCreateFlags flags, const unsigned int& count, std::array<VkFence, Count>& fences) {
    VkFenceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = flags};
    for (unsigned int i = 0; i < count; i++) {
        VkResult result = vkCreateFence(device, &createInfo, nullptr, &fences[i]);
        assert(result == VK_SUCCESS);
    }
}

template <size_t Count>
inline void createDescriptorSetLayout(VkDevice device, const std::array<uint32_t, Count>& counts, const std::array<VkDescriptorType, Count>& types, const std::array<VkShaderStageFlags, Count>& stageFlags, VkDescriptorSetLayout& descriptorSetLayout) {
    std::array<VkDescriptorSetLayoutBinding, Count> bindings;
    for (unsigned int i = 0; i < Count; i++) {
        bindings[i] = VkDescriptorSetLayoutBinding{
            .binding = i,
            .descriptorType = types[i],
            .descriptorCount = counts[i],
            .stageFlags = stageFlags[i],
            .pImmutableSamplers = nullptr};
    }

    VkDescriptorSetLayoutCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = static_cast<uint32_t>(bindings.size()),
        .pBindings = bindings.data()};

    VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &descriptorSetLayout);
    assert(result == VK_SUCCESS);
}

template <size_t Count>
inline void createPipelineLayout(VkDevice device, const std::array<VkDescriptorSetLayout, Count>& descriptorSetLayouts, VkPipelineLayout& pipelineLayout) {
    VkPipelineLayoutCreateInfo layoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<uint32_t>(Count),
        .pSetLayouts = descriptorSetLayouts.data()};

    VkResult result = vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, &pipelineLayout);
    assert(result == VK_SUCCESS);
}

template <size_t Count>
inline void createGraphicsPipeline(VkDevice device, const std::vector<uint32_t>& vertexShader, const std::vector<uint32_t>& fragmentShader, const uint32_t& vertexInputStride, const std::array<VkFormat, Count>& vertexInputAttributeFormats, const std::array<uint32_t, Count>& vertexInputAttributeOffsets, VkExtent2D swapChainExtent, VkPipelineLayout pipelineLayout, VkRenderPass renderPass, VkPipeline& pipeline) {
    VkShaderModule vertexShaderModule = createShaderModule(device, vertexShader);
    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertexShaderModule,
        .pName = "main"};
    VkShaderModule fragmentShaderModule = createShaderModule(device, fragmentShader);
    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragmentShaderModule,
        .pName = "main"};
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageCreateInfos = {vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

    VkVertexInputBindingDescription vertexInputBindingDescription{
        .binding = 0,
        .stride = vertexInputStride,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX};
    std::array<VkVertexInputAttributeDescription, Count> vertexInputAttributeDescriptions;
    for (unsigned int i = 0; i < vertexInputAttributeDescriptions.size(); i++)
        vertexInputAttributeDescriptions[i] = {
            .binding = 0,
            .location = i,
            .format = vertexInputAttributeFormats[i],
            .offset = vertexInputAttributeOffsets[i]};

    VkPipelineVertexInputStateCreateInfo vertexInputStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &vertexInputBindingDescription,
        .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributeDescriptions.size()),
        .pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data()};

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE};

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapChainExtent.width),
        .height = static_cast<float>(swapChainExtent.height),
        .minDepth = 0.0f,
        .maxDepth = 1.0f};

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapChainExtent};

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = &viewport,
        .scissorCount = 1,
        .pScissors = &scissor};

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE};

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT};

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = VK_FALSE};

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState};

    VkGraphicsPipelineCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = static_cast<uint32_t>(shaderStageCreateInfos.size()),
        .pStages = shaderStageCreateInfos.data(),
        .pVertexInputState = &vertexInputStageCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = nullptr,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE};

    VkResult result = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, &pipeline);
    assert(result == VK_SUCCESS);

    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(device, vertexShaderModule, nullptr);

    // TODO: TEMP 可能要销毁
}

inline void createBuffer(VmaAllocator allocator, VkDeviceSize bufferSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation) {
    VkBufferCreateInfo bufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = bufferSize,
        .usage = bufferUsage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE};

    VmaAllocationCreateInfo allocationCreateInfo{.usage = memoryUsage};
    VkResult result = vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr);
    assert(result == VK_SUCCESS);
}

template <size_t Count>
inline void createDescriptorPool(VkDevice device, const std::array<VkDescriptorType, Count>& types, const std::array<uint32_t, Count>& descriptorCounts, const uint32_t& descriptorSetCount, VkDescriptorPool& descriptorPool) {
    std::array<VkDescriptorPoolSize, Count> poolSizes;
    for (unsigned int i = 0; i < Count; i++)
        poolSizes[i] = VkDescriptorPoolSize{
            .type = types[i],
            .descriptorCount = descriptorCounts[i]};
    VkDescriptorPoolCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,  // TODO:
        .maxSets = descriptorSetCount,
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data()};
    VkResult result = vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool);
    assert(result == VK_SUCCESS);
}

inline void allocateDescriptorSet(VkDevice device, VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool, VkDescriptorSet& descriptorSet) {
    VkDescriptorSetAllocateInfo allocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &descriptorSetLayout};

    VkResult result = vkAllocateDescriptorSets(device, &allocateInfo, &descriptorSet);
    assert(result == VK_SUCCESS);
}

template <size_t Count>
inline void updateUniformDescriptorSet(VkDevice device, VkDescriptorSet descriptorSet, const std::array<VkBuffer, Count>& uniformBuffers, const std::array<VkDeviceSize, Count>& uniformBufferRanges) {
    std::array<VkWriteDescriptorSet, Count> writeDescriptorSets;
    std::array<VkDescriptorBufferInfo, Count> bufferInfos;
    for (unsigned int i = 0; i < Count; i++) {
        bufferInfos[i] = VkDescriptorBufferInfo{
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = uniformBufferRanges[i]};
        writeDescriptorSets[i] = VkWriteDescriptorSet{
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = descriptorSet,
            .dstBinding = i,
            .dstArrayElement = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .pBufferInfo = &bufferInfos[i],
            .pImageInfo = nullptr,
            .pTexelBufferView = nullptr};
    }
    vkUpdateDescriptorSets(device, Count, writeDescriptorSets.data(), 0, nullptr);
}

inline void copyBuffer(VmaAllocator allocator, VkCommandBuffer commandBuffer, VkBuffer stagingBuffer, VmaAllocation stagingAllocation, const void* data, VkDeviceSize size, VkBuffer dstBuffer) {
    void* mappedData;
    vmaMapMemory(allocator, stagingAllocation, &mappedData);
    memcpy(mappedData, data, size);
    vmaUnmapMemory(allocator, stagingAllocation);
    vmaFlushAllocation(allocator, stagingAllocation, 0, size);

    VkBufferCopy region{.size = size};
    vkCmdCopyBuffer(commandBuffer, stagingBuffer, dstBuffer, 1, &region);
    VkBufferMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT | VK_ACCESS_INDEX_READ_BIT,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = dstBuffer,
        .size = VK_WHOLE_SIZE};
    vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, 0, 0, nullptr, 1, &barrier, 0, nullptr);
}

}  // namespace MelonFrontend
