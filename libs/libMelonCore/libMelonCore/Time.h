#pragma once

#include <chrono>

namespace MelonCore {

class Time {
  public:
    static Time* instance();

    float const& deltaTime() const;

  private:
    void initialize();
    void update();

    std::chrono::steady_clock::time_point m_PreviousTimePoint;
    float m_DeltaTime{};

    friend class Instance;
};

}  // namespace MelonCore
