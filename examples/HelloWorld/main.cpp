#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <cstdio>
#include <memory>

class HelloWorldSystem : public Melon::SystemBase {
  public:
    HelloWorldSystem(const unsigned int& a, const unsigned int& b) : m_A(a), m_B(b), m_Sum(a + b) {}

  protected:
    void onEnter() override {}

    void onUpdate() override {
        std::printf("Delta time : %f, %u + %u = %u\n", time()->deltaTime(), m_A, m_B, m_Sum);
        std::shared_ptr<Melon::TaskHandle> a = taskManager()->schedule([this]() { m_A++; }, {predecessor()});
        std::shared_ptr<Melon::TaskHandle> b = taskManager()->schedule([this]() { m_B++; }, {predecessor()});
        predecessor() = taskManager()->schedule([this]() { m_Sum = m_A + m_B; }, {a, b});
        if (m_Sum >= 100)
            instance()->quit();
    }

    void onExit() override {}

  private:
    unsigned int m_A;
    unsigned int m_B;
    unsigned int m_Sum;
};

int main() {
    Melon::Instance()
        .registerSystem<HelloWorldSystem>(1U, 2U)
        .start();
    return 0;
}