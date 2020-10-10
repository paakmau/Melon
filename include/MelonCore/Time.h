#pragma once

#include <chrono>

namespace Melon {

class Time {
   public:
    static Time* instance();

    const float& deltaTime() const;

   private:
    void init();
    void update();

    std::chrono::steady_clock::time_point _previousTimePoint;
    float _deltaTime{};

    friend class Instance;
};

}  // namespace Melon
