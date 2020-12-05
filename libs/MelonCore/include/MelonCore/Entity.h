#pragma once

#include <climits>

namespace MelonCore {

class Archetype;

struct Entity {
    static constexpr unsigned int k_InvalidId = std::numeric_limits<unsigned int>::max();

    static constexpr Entity invalidEntity() { return Entity{k_InvalidId}; }

    bool operator==(Entity const& other) const {
        return id == other.id;
    }

    bool valid() const { return id != k_InvalidId; }

    unsigned int id;
};

}  // namespace MelonCore
