#include <MelonFrontend/UniformBufferPool.h>

namespace MelonFrontend {

void FixedSizeBufferPool::initialize(VkDevice device, VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage) {
    _device = device;
    _allocator = allocator;
    _size = size;
    _bufferUsage = bufferUsage;
    _memoryUsage = memoryUsage;

    for (unsigned int i = 0; i < _pool.size(); i++) {
        Buffer buffer;
        createBuffer(_allocator, _size, _bufferUsage, _memoryUsage, buffer.buffer, buffer.allocation);
        _pool[i] = buffer;
    }
}

void FixedSizeBufferPool::terminate() {
    for (const Buffer& buffer : _pool)
        vmaDestroyBuffer(_allocator, buffer.buffer, buffer.allocation);
}

Buffer FixedSizeBufferPool::request() {
    if (_poolCount == 0) {
        Buffer buffer;
        createBuffer(_allocator, _size, _bufferUsage, _memoryUsage, buffer.buffer, buffer.allocation);
        return buffer;
    }
    return _pool[--_poolCount];
}

void FixedSizeBufferPool::recycle(const Buffer& buffer) {
    if (_poolCount == _pool.size())
        vmaDestroyBuffer(_allocator, buffer.buffer, buffer.allocation);
    else
        _pool[_poolCount++] = buffer;
}

void UniformBufferPool::initialize(VkDevice device, VmaAllocator allocator, VkDescriptorSetLayout layout, VkDescriptorPool descriptorPool) { _device = device, _allocator = allocator, _layout = layout, _descriptorPool = descriptorPool; }

void UniformBufferPool::terminate() {
    for (FixedSizeBufferPool pool : _stagingBufferPools)
        pool.terminate();
    for (FixedSizeBufferPool pool : _bufferPools)
        pool.terminate();
}

void UniformBufferPool::registerUniformObjectSize(VkDeviceSize size) {
    if (_sizeIndexMap.contains(size)) return;
    _stagingBufferPools.emplace_back(FixedSizeBufferPool());
    _stagingBufferPools.back().initialize(_device, _allocator, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
    _bufferPools.emplace_back(FixedSizeBufferPool());
    _bufferPools.back().initialize(_device, _allocator, size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
    _sizeIndexMap.emplace(size, _bufferPools.size() - 1);
}

UniformBuffer UniformBufferPool::request(VkDeviceSize size) {
    const unsigned int& index = _sizeIndexMap[size];
    Buffer stagingBuffer = _stagingBufferPools[index].request();
    Buffer buffer = _bufferPools[index].request();

    VkDescriptorSet descriptorSet;
    allocateDescriptorSet(_device, _layout, _descriptorPool, descriptorSet);
    updateUniformDescriptorSet<1>(_device, descriptorSet, {buffer.buffer}, {size});

    return UniformBuffer{size, Buffer{stagingBuffer.buffer, stagingBuffer.allocation}, Buffer{buffer.buffer, buffer.allocation}, descriptorSet};
}

void UniformBufferPool::recycle(UniformBuffer buffer) {
    const unsigned int& index = _sizeIndexMap[buffer.size];
    _stagingBufferPools[index].recycle(buffer.stagingBuffer);
    _bufferPools[index].recycle(buffer.buffer);

    vkFreeDescriptorSets(_device, _descriptorPool, 1, &buffer.descriptorSet);
}

}  // namespace MelonFrontend
