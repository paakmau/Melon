#pragma once

#include <MelonCore/EntityManager.h>
#include <MelonCore/World.h>

#include <memory>

namespace MelonCore {

class EntityManager;
class World;

class Instance {
  public:
    char const* const& applicationName() const { return m_ApplicationName; }
    char const*& applicationName() { return m_ApplicationName; }

    template <typename Type, typename... Args>
    void registerSystem(Args&&... args);

    void start();
    void quit();

    static Instance* instance();

  private:
    Instance();

    void mainLoop();

    char const* m_ApplicationName{};

    std::unique_ptr<EntityManager> m_EntityManager;
    std::unique_ptr<World> m_DefaultWorld;

    bool m_ShouldQuit{};
};

template <typename Type, typename... Args>
void Instance::registerSystem(Args&&... args) {
    m_DefaultWorld->registerSystem<Type>(std::forward<Args>(args)...);
}

}  // namespace MelonCore
