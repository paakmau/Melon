#include <MelonCore/Time.h>

namespace MelonCore {

Time* Time::instance() {
    static Time sInstance;
    return &sInstance;
}

float const& Time::deltaTime() const {
    return m_DeltaTime;
}

void Time::initialize() {
    m_PreviousTimePoint = std::chrono::steady_clock::now();
}

void Time::update() {
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    long long duration = std::chrono::duration_cast<std::chrono::microseconds>(now - m_PreviousTimePoint).count();
    m_DeltaTime = static_cast<float>(static_cast<double>(duration) / static_cast<double>(std::chrono::microseconds::period::den));
    m_PreviousTimePoint = now;
}

}  // namespace MelonCore
