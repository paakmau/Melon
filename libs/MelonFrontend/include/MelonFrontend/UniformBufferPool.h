#pragma once

#include <MelonFrontend/UniformBuffer.h>
#include <MelonFrontend/VulkanPlatform.h>
#include <MelonFrontend/VulkanUtil.h>

#include <array>
#include <unordered_map>
#include <utility>
#include <vector>

namespace MelonFrontend {

class FixedSizeBufferPool {
   public:
    static constexpr unsigned int kPoolSize = 2048U;

    void initialize(VkDevice device, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
    // FixedSizeBufferPool can't terminate util all buffers are recycled
    void terminate();

    Buffer request();
    void recycle(const Buffer& buffer);

    const VkDevice& device() const { return _device; }
    const VkDeviceSize& size() const { return _size; }

   private:
    VkDevice _device;
    VmaAllocator _allocator;
    VkDeviceSize _size;
    VkBufferUsageFlags _bufferUsage;
    VmaMemoryUsage _memoryUsage;

    std::array<Buffer, kPoolSize> _pool;
    // std::vector<Buffer> _extendedPool;
    unsigned int _poolCount = kPoolSize;
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
    VkDevice _device;
    VmaAllocator _allocator;
    std::vector<FixedSizeBufferPool> _stagingBufferPools;
    std::vector<FixedSizeBufferPool> _bufferPools;
    std::unordered_map<VkDeviceSize, unsigned int> _sizeIndexMap;
    VkDescriptorSetLayout _layout;
    VkDescriptorPool _descriptorPool;
};

}  // namespace MelonFrontend
