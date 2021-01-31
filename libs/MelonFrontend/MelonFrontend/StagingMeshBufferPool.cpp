#include <MelonFrontend/StagingMeshBufferPool.h>
#include <MelonFrontend/VulkanUtils.h>

namespace Melon {

void VariableSizeBufferPool::initialize(VmaAllocator allocator, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage) { m_Allocator = allocator, m_BufferUsage = bufferUsage, m_MemoryUsage = memoryUsage; }

void VariableSizeBufferPool::terminate() {
    for (const auto& [bufferSize, buffer] : m_FreeBuffers)
        vmaDestroyBuffer(m_Allocator, buffer.buffer.buffer, buffer.buffer.allocation);
}

VariableSizeBuffer VariableSizeBufferPool::request(VkDeviceSize size) {
    VariableSizeBuffer buffer;
    // Check if a buffer with a sufficiently large size exists
    auto iter = m_FreeBuffers.lower_bound(size);
    if (iter != m_FreeBuffers.end()) {
        buffer = iter->second;
        m_FreeBuffers.erase(iter);
        return buffer;
    }
    // Create a new buffer
    createBuffer(m_Allocator, size, m_BufferUsage, m_MemoryUsage, buffer.buffer.buffer, buffer.buffer.allocation);
    buffer.lastAccessedFrame = m_CurrentFrame;
    buffer.size = size;
    return buffer;
}

void VariableSizeBufferPool::recycle(VariableSizeBuffer buffer) {
    buffer.lastAccessedFrame = m_CurrentFrame;
    m_FreeBuffers.emplace(buffer.size, buffer);
}

void VariableSizeBufferPool::collectGarbage() {
    m_CurrentFrame++;
    std::multimap<VkDeviceSize, VariableSizeBuffer> buffers;
    buffers.swap(m_FreeBuffers);
    for (const auto& [bufferSize, buffer] : buffers)
        if (buffer.lastAccessedFrame + k_MaxSurvivalFrameCount < m_CurrentFrame)
            vmaDestroyBuffer(m_Allocator, buffer.buffer.buffer, buffer.buffer.allocation);
        else
            m_FreeBuffers.emplace(bufferSize, buffer);
}

void StagingMeshBufferPool::initialize(VmaAllocator allocator) {
    m_Allocator = allocator;
    m_StagingVertexBufferPool.initialize(m_Allocator, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    m_StagingIndexBufferPool.initialize(m_Allocator, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
}

void StagingMeshBufferPool::terminate() {
    m_StagingVertexBufferPool.terminate();
    m_StagingIndexBufferPool.terminate();
}

StagingMeshBuffer StagingMeshBufferPool::request(const VkDeviceSize& vertexBufferSize, const VkDeviceSize& indexBufferSize) {
    VariableSizeBuffer vertexBuffer = m_StagingVertexBufferPool.request(vertexBufferSize);
    VariableSizeBuffer indexBuffer = m_StagingIndexBufferPool.request(indexBufferSize);
    return StagingMeshBuffer{vertexBuffer, indexBuffer};
}

void StagingMeshBufferPool::recycle(const StagingMeshBuffer& buffer) {
    m_StagingVertexBufferPool.recycle(buffer.vertexBuffer);
    m_StagingIndexBufferPool.recycle(buffer.indexBuffer);
}

void StagingMeshBufferPool::collectGarbage() {
    m_StagingVertexBufferPool.collectGarbage();
    m_StagingIndexBufferPool.collectGarbage();
}

}  // namespace Melon
