#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/TaskHandle.h>
#include <MelonCore/TaskManager.h>
#include <MelonCore/Time.h>

#include <cstdio>
#include <memory>

class HelloWorldSystem : public MelonCore::SystemBase {
   public:
    HelloWorldSystem(int numA) : _numA(numA), _numB(0), _numSum(_numA + _numB) {}

    void onEnter() override {}

    void onUpdate() override {
        std::printf("Delta time : %f, %d + %d = %d\n", MelonCore::Time::instance()->deltaTime(), _numA, _numB, _numSum);
        std::shared_ptr<MelonCore::TaskHandle> a = MelonCore::TaskManager::instance()->schedule([this]() { _numA++; }, {predecessor()});
        std::shared_ptr<MelonCore::TaskHandle> b = MelonCore::TaskManager::instance()->schedule([this]() { _numB++; }, {predecessor()});
        predecessor() = MelonCore::TaskManager::instance()->schedule([this]() { _numSum = _numA + _numB; }, {a, b});
    }

    void onExit() override {}

    int _numA;
    int _numB;
    int _numSum;
};

int main() {
    MelonCore::Instance instance;
    instance.registerSystem<HelloWorldSystem>(1);
    instance.startGame();
    return 0;
}