#pragma once

#include <MelonCore/Entity.h>

#include <cstddef>
#include <cstdlib>
#include <unordered_map>
#include <vector>

namespace MelonCore {

struct ChunkLayout {
    unsigned int capacity;
    std::size_t entityOffset{};
    std::unordered_map<unsigned int, unsigned int> componentIndexMap;
    std::vector<std::size_t> componentSizes;
    std::vector<std::size_t> componentOffsets;
};

struct Chunk {
    static constexpr std::size_t k_Align = 64;
    static constexpr std::size_t k_Size = 16 << 10;

    Chunk() {}
    Chunk(Chunk const&) = delete;

    std::aligned_storage_t<k_Size, k_Align> memory;
};

}  // namespace MelonCore
