#include <MelonCore/EventManager.h>
#include <MelonCore/Instance.h>
#include <MelonCore/SystemBase.h>
#include <MelonCore/Time.h>
#include <MelonFrontend/InputEvents.h>
#include <MelonFrontend/RenderSystem.h>

#include <cstdio>
#include <memory>
#include <string>

class PrintInputSystem : public Melon::SystemBase {
  protected:
    void onEnter() final {}
    void onUpdate() final {
        for (auto it = eventManager()->begin<Melon::KeyDownEvent>(); it != eventManager()->end<Melon::KeyDownEvent>(); it++)
            printf("Key pressed: %d\n", it->key);
    }
    void onExit() final {}
};

int main() {
    Melon::Instance()
        .setApplicationName("Print Input")
        .registerSystem<Melon::RenderSystem>(800, 600)
        .registerSystem<PrintInputSystem>()
        .start();
    return 0;
}
