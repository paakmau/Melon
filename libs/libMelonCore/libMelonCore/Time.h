#pragma once

#include <chrono>

namespace Melon {

class Time {
  public:
    float const& deltaTime() const;

  private:
    Time() {}

    void initialize();
    void update();

    std::chrono::steady_clock::time_point m_PreviousTimePoint;
    float m_DeltaTime{};

    friend class Instance;
};

}  // namespace Melon
