#include <MelonCore/Time.h>

namespace MelonCore {

Time* Time::instance() {
    static Time sInstance;
    return &sInstance;
}

float const& Time::deltaTime() const {
    return _deltaTime;
}

void Time::initialize() {
    _previousTimePoint = std::chrono::steady_clock::now();
}

void Time::update() {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    long long duration = std::chrono::duration_cast<std::chrono::microseconds>(now - _previousTimePoint).count();
    _deltaTime = static_cast<float>(static_cast<double>(duration) / static_cast<double>(std::chrono::microseconds::period::den));
    _previousTimePoint = now;
}

}  // namespace MelonCore
