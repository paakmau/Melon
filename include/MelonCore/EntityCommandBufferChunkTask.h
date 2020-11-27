#pragma once

#include <MelonCore/EntityManager.h>
#include <MelonCore/ChunkAccessor.h>

namespace MelonCore {

class EntityCommandBufferChunkTask {
   public:
    virtual void execute(const ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex, EntityCommandBuffer* entityCommandBuffer) = 0;
};

}  // namespace MelonCore
