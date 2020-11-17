#pragma once

#include <memory>
#include <vector>

namespace MelonCore {

class ChunkAccessor;

class ChunkTask {
   public:
    virtual void execute(const ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) = 0;
};

}  // namespace MelonCore
