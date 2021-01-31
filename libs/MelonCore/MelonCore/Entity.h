#pragma once

#include <climits>

namespace Melon {

class Archetype;

struct Entity {
    static constexpr unsigned int k_InvalidId = std::numeric_limits<unsigned int>::max();

    static constexpr Entity invalidEntity() { return Entity{k_InvalidId}; }

    bool operator==(const Entity& other) const {
        return id == other.id;
    }

    bool valid() const { return id != k_InvalidId; }

    unsigned int id;
};

}  // namespace Melon
