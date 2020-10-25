#pragma once

#include <vector>
#include <bitset>

namespace MelonCore {

struct EntityFilter {
    std::bitset<1024> componentMask;
};

}  // namespace MelonCore
