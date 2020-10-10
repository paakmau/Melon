#pragma once

#include <vector>
#include <bitset>

namespace Melon {

struct EntityFilter {
    std::bitset<1024> componentMask;
};

}  // namespace Melon
