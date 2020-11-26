#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/TaskHandle.h>
#include <MelonCore/TaskManager.h>
#include <MelonCore/Time.h>

#include <cstdio>
#include <memory>

class HelloWorldSystem : public MelonCore::SystemBase {
   public:
    HelloWorldSystem(const unsigned int& a, const unsigned int& b) : _a(a), _b(b), _sum(a + b) {}

   protected:
    void onEnter() override {}

    void onUpdate() override {
        std::printf("Delta time : %f, %u + %u = %u\n", MelonCore::Time::instance()->deltaTime(), _a, _b, _sum);
        std::shared_ptr<MelonCore::TaskHandle> a = MelonCore::TaskManager::instance()->schedule([this]() { _a++; }, {predecessor()});
        std::shared_ptr<MelonCore::TaskHandle> b = MelonCore::TaskManager::instance()->schedule([this]() { _b++; }, {predecessor()});
        predecessor() = MelonCore::TaskManager::instance()->schedule([this]() { _sum = _a + _b; }, {a, b});
        if (_sum >= 100)
            MelonCore::Instance::instance()->quit();
    }

    void onExit() override {}

   private:
    unsigned int _a;
    unsigned int _b;
    unsigned int _sum;
};

int main() {
    MelonCore::Instance::instance()->registerSystem<HelloWorldSystem>(1U, 2U);
    MelonCore::Instance::instance()->start();
    return 0;
}