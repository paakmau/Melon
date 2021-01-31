#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonTask/TaskHandle.h>
#include <MelonTask/TaskManager.h>

#include <cstdio>
#include <memory>

class HelloWorldSystem : public Melon::SystemBase {
  public:
    HelloWorldSystem(const unsigned int& a, const unsigned int& b) : _a(a), _b(b), _sum(a + b) {}

  protected:
    void onEnter() override {}

    void onUpdate() override {
        std::printf("Delta time : %f, %u + %u = %u\n", time()->deltaTime(), _a, _b, _sum);
        std::shared_ptr<Melon::TaskHandle> a = taskManager()->schedule([this]() { _a++; }, {predecessor()});
        std::shared_ptr<Melon::TaskHandle> b = taskManager()->schedule([this]() { _b++; }, {predecessor()});
        predecessor() = taskManager()->schedule([this]() { _sum = _a + _b; }, {a, b});
        if (_sum >= 100)
            instance()->quit();
    }

    void onExit() override {}

  private:
    unsigned int _a;
    unsigned int _b;
    unsigned int _sum;
};

int main() {
    Melon::Instance instance;
    instance.registerSystem<HelloWorldSystem>(1U, 2U);
    instance.start();
    return 0;
}