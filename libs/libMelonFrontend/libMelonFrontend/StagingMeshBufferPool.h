#pragma once

#include <libMelonFrontend/Buffer.h>
#include <libMelonFrontend/MeshBuffer.h>
#include <libMelonFrontend/VulkanPlatform.h>

#include <map>
#include <vector>

namespace Melon {

struct VariableSizeBuffer {
    Buffer buffer;
    VkDeviceSize size;
    uint64_t lastAccessedFrame;
};

class VariableSizeBufferPool {
  public:
    void initialize(VmaAllocator allocator, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage);
    void terminate();

    VariableSizeBuffer request(VkDeviceSize size);
    void recycle(VariableSizeBuffer buffer);

    void collectGarbage();

  private:
    static constexpr uint64_t k_MaxSurvivalFrameCount = 3U;

    VmaAllocator m_Allocator;
    VkBufferUsageFlags m_BufferUsage;
    VmaMemoryUsage m_MemoryUsage;

    std::multimap<VkDeviceSize, VariableSizeBuffer> m_FreeBuffers;
    uint64_t m_CurrentFrame{};
};

struct StagingMeshBuffer {
    VariableSizeBuffer vertexBuffer;
    VariableSizeBuffer indexBuffer;
};

class StagingMeshBufferPool {
  public:
    void initialize(VmaAllocator allocator);
    void terminate();

    StagingMeshBuffer request(VkDeviceSize const& vertexBufferSize, VkDeviceSize const& indexBufferSize);
    void recycle(StagingMeshBuffer const& buffer);

    void collectGarbage();

  private:
    VmaAllocator m_Allocator;
    VariableSizeBufferPool m_StagingVertexBufferPool;
    VariableSizeBufferPool m_StagingIndexBufferPool;
};

}  // namespace Melon
