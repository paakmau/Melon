#pragma once

#include <memory>
#include <vector>

namespace Melon {

class ChunkAccessor;

class ChunkTask {
   public:
    virtual void execute(const ChunkAccessor& chunkAccessor) = 0;
};

}  // namespace Melon
