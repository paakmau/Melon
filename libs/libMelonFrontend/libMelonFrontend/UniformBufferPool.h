#pragma once

#include <libMelonFrontend/UniformBuffer.h>
#include <libMelonFrontend/VulkanPlatform.h>
#include <libMelonFrontend/VulkanUtils.h>

#include <array>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MelonFrontend {

class FixedSizeBufferPool {
  public:
    static constexpr unsigned int k_PoolSize = 2048U;

    void initialize(VkDevice device, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
    // FixedSizeBufferPool can't terminate util all buffers are recycled
    void terminate();

    Buffer request();
    void recycle(Buffer const& buffer);

    VkDevice const& device() const { return m_Device; }
    VkDeviceSize const& size() const { return m_Size; }

  private:
    VkDevice m_Device;
    VmaAllocator m_Allocator;
    VkDeviceSize m_Size;
    VkBufferUsageFlags m_BufferUsage;
    VmaMemoryUsage m_MemoryUsage;

    std::array<Buffer, k_PoolSize> m_Pool;
    // std::vector<Buffer> m_ExtendedPool;
    unsigned int m_PoolCount = k_PoolSize;
};

class UniformBufferPool {
  public:
    void initialize(VkDevice device, VmaAllocator allocator, VkDescriptorSetLayout layout, VkDescriptorPool descriptorPool);
    // UniformBufferPool can't terminate util all buffers are recycled
    void terminate();

    void registerUniformObjectSize(VkDeviceSize size);

    UniformBuffer request(VkDeviceSize size);
    void recycle(UniformBuffer memory);

  private:
    VkDevice m_Device;
    VmaAllocator m_Allocator;
    std::vector<FixedSizeBufferPool> m_StagingBufferPools;
    std::vector<FixedSizeBufferPool> m_BufferPools;
    std::unordered_map<VkDeviceSize, unsigned int> m_SizeIndexMap;
    VkDescriptorSetLayout m_Layout;
    VkDescriptorPool m_DescriptorPool;
};

}  // namespace MelonFrontend
