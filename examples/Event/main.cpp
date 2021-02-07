#include <MelonCore/Event.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>

#include <cstdio>
#include <memory>
#include <string>

struct Hello : public Melon::Event {
    std::string message;
};

class SenderSystem : public Melon::SystemBase {
  protected:
    void onEnter() final {}
    void onUpdate() final {
        eventManager()->send<Hello>(Hello{.message = "Hello!"});
        if (m_Counter++ > 1000)
            instance()->quit();
    }
    void onExit() final {}

  private:
    unsigned int m_Counter{};
};

class ReceiverSystem : public Melon::SystemBase {
  protected:
    void onEnter() final {}
    void onUpdate() final {
        for (auto it = eventManager()->begin<Hello>(); it != eventManager()->end<Hello>(); it++)
            printf("Receive: %s\n", it->message.c_str());
    }
    void onExit() final {}
};

int main() {
    Melon::Instance()
        .registerSystem<SenderSystem>()
        .registerSystem<ReceiverSystem>()
        .start();
    return 0;
}