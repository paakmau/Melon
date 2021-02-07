#pragma once

#include <MelonCore/EntityManager.h>
#include <MelonCore/ResourceManager.h>
#include <MelonCore/Time.h>
#include <MelonCore/World.h>
#include <MelonTask/TaskManager.h>

#include <memory>
#include <string>

namespace Melon {

class EntityManager;
class World;

class Instance {
  public:
    Instance();

    Instance& setApplicationName(const std::string& applicationName) {
        m_ApplicationName = applicationName;
        return *this;
    }

    template <typename Type, typename... Args>
    Instance& registerSystem(Args&&... args);

    void start();
    void quit();

    const std::string& applicationName() const { return m_ApplicationName; }
    std::string& applicationName() { return m_ApplicationName; }

  private:
    void mainLoop();

    std::string m_ApplicationName{};

    TaskManager m_TaskManager;

    Time m_Time;

    ResourceManager m_ResourceManager;

    std::unique_ptr<World> m_DefaultWorld;

    bool m_ShouldQuit{};
};

template <typename Type, typename... Args>
Instance& Instance::registerSystem(Args&&... args) {
    m_DefaultWorld->registerSystem<Type>(std::forward<Args>(args)...);
    return *this;
}

}  // namespace Melon
