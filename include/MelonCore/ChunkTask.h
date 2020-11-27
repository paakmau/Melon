#pragma once

#include <MelonCore/ChunkAccessor.h>

namespace MelonCore {

class ChunkTask {
   public:
    virtual void execute(const ChunkAccessor& chunkAccessor, const unsigned int& chunkIndex, const unsigned int& firstEntityIndex) = 0;
};

}  // namespace MelonCore
