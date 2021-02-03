#pragma once

#include <MelonFrontend/Buffer.h>
#include <MelonFrontend/UniformBuffer.h>
#include <MelonFrontend/VulkanPlatform.h>

#include <array>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Melon {

class FixedSizeBufferPool {
  public:
    static constexpr unsigned int k_MaxFreeBufferCount = 2048U;

    void initialize(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
    // FixedSizeBufferPool can't terminate util all buffers are recycled
    void terminate();

    Buffer request();
    void recycle(const Buffer& buffer);

    const VkDeviceSize& size() const { return m_Size; }

  private:
    VmaAllocator m_Allocator;
    VkDeviceSize m_Size;
    VkBufferUsageFlags m_BufferUsage;
    VmaMemoryUsage m_MemoryUsage;

    std::array<Buffer, k_MaxFreeBufferCount> m_FreeBuffers;
    unsigned int m_FreeBufferCount = k_MaxFreeBufferCount;
};

class UniformBufferPool {
  public:
    void initialize(VkDevice device, VmaAllocator allocator, VkDescriptorPool descriptorPool);
    // UniformBufferPool can't terminate util all buffers are recycled
    void terminate();

    void registerUniformObjectSize(VkDeviceSize size);

    UniformBuffer request(VkDescriptorSetLayout layout, VkDeviceSize size);
    void recycle(UniformBuffer memory);

  private:
    VkDevice m_Device;
    VmaAllocator m_Allocator;
    std::vector<FixedSizeBufferPool> m_StagingBufferPools;
    std::vector<FixedSizeBufferPool> m_BufferPools;
    std::unordered_map<VkDeviceSize, unsigned int> m_SizeIndexMap;
    VkDescriptorPool m_DescriptorPool;
};

}  // namespace Melon
