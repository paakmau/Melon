#pragma once

#include <memory>
#include <vector>

namespace MelonCore {

class ChunkAccessor;

class ChunkTask {
   public:
    virtual void execute(const ChunkAccessor& chunkAccessor) = 0;
};

}  // namespace MelonCore
