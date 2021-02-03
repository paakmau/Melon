#include <MelonFrontend/UniformBufferPool.h>
#include <MelonFrontend/VulkanUtils.h>

namespace Melon {

void FixedSizeBufferPool::initialize(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage) {
    m_Allocator = allocator;
    m_Size = size;
    m_BufferUsage = bufferUsage;
    m_MemoryUsage = memoryUsage;

    for (unsigned int i = 0; i < m_FreeBuffers.size(); i++) {
        Buffer buffer;
        createBuffer(m_Allocator, m_Size, m_BufferUsage, m_MemoryUsage, buffer.buffer, buffer.allocation);
        m_FreeBuffers[i] = buffer;
    }
}

void FixedSizeBufferPool::terminate() {
    for (const Buffer& buffer : m_FreeBuffers)
        vmaDestroyBuffer(m_Allocator, buffer.buffer, buffer.allocation);
}

Buffer FixedSizeBufferPool::request() {
    if (m_FreeBufferCount == 0) {
        Buffer buffer;
        createBuffer(m_Allocator, m_Size, m_BufferUsage, m_MemoryUsage, buffer.buffer, buffer.allocation);
        return buffer;
    }
    return m_FreeBuffers[--m_FreeBufferCount];
}

void FixedSizeBufferPool::recycle(const Buffer& buffer) {
    if (m_FreeBufferCount == m_FreeBuffers.size())
        vmaDestroyBuffer(m_Allocator, buffer.buffer, buffer.allocation);
    else
        m_FreeBuffers[m_FreeBufferCount++] = buffer;
}

void UniformBufferPool::initialize(VkDevice device, VmaAllocator allocator, VkDescriptorPool descriptorPool) { m_Device = device, m_Allocator = allocator, m_DescriptorPool = descriptorPool; }

void UniformBufferPool::terminate() {
    for (FixedSizeBufferPool pool : m_StagingBufferPools)
        pool.terminate();
    for (FixedSizeBufferPool pool : m_BufferPools)
        pool.terminate();
}

void UniformBufferPool::registerUniformObjectSize(VkDeviceSize size) {
    if (m_SizeIndexMap.contains(size)) return;
    m_StagingBufferPools.emplace_back(FixedSizeBufferPool());
    m_StagingBufferPools.back().initialize(m_Allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    m_BufferPools.emplace_back(FixedSizeBufferPool());
    m_BufferPools.back().initialize(m_Allocator, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    m_SizeIndexMap.emplace(size, m_BufferPools.size() - 1);
}

UniformBuffer UniformBufferPool::request(VkDescriptorSetLayout layout, VkDeviceSize size) {
    const unsigned int& index = m_SizeIndexMap[size];
    Buffer stagingBuffer = m_StagingBufferPools[index].request();
    Buffer buffer = m_BufferPools[index].request();

    VkDescriptorSet descriptorSet;
    allocateDescriptorSet(m_Device, layout, m_DescriptorPool, descriptorSet);
    updateUniformDescriptorSet<1>(m_Device, descriptorSet, {buffer.buffer}, {size});

    return UniformBuffer{size, Buffer{stagingBuffer.buffer, stagingBuffer.allocation}, Buffer{buffer.buffer, buffer.allocation}, descriptorSet};
}

void UniformBufferPool::recycle(UniformBuffer buffer) {
    const unsigned int& index = m_SizeIndexMap[buffer.size];
    m_StagingBufferPools[index].recycle(buffer.stagingBuffer);
    m_BufferPools[index].recycle(buffer.buffer);

    vkFreeDescriptorSets(m_Device, m_DescriptorPool, 1, &buffer.descriptorSet);
}

}  // namespace Melon
