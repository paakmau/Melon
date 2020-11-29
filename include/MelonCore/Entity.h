#pragma once

#include <climits>

namespace MelonCore {

class Archetype;

struct Entity {
    static constexpr unsigned int kInvalidId = std::numeric_limits<unsigned int>::max();

    static constexpr Entity invalidEntity() { return Entity{kInvalidId}; }

    bool operator==(const Entity& other) const {
        return id == other.id;
    }

    bool valid() const { return id != kInvalidId; }

    unsigned int id;
};

}  // namespace MelonCore
