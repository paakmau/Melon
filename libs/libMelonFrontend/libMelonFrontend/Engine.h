#pragma once

#include <libMelonFrontend/MeshBuffer.h>
#include <libMelonFrontend/RenderBatch.h>
#include <libMelonFrontend/Renderer.h>
#include <libMelonFrontend/SwapChain.h>
#include <libMelonFrontend/Window.h>
#include <libMelonTask/TaskManager.h>

#include <cstddef>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

namespace Melon {

class Engine {
  public:
    void initialize(MelonTask::TaskManager* taskManager, char const* title, unsigned int const& width, unsigned int const& height);
    void terminate();

    void beginFrame();
    MeshBuffer createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    void destroyMeshBuffer(MeshBuffer const& meshBuffer);
    void beginBatches();
    void addBatch(std::vector<glm::mat4> const& models, MeshBuffer const& meshBuffer);
    void endBatches();
    void renderFrame(glm::mat4 const& vp);
    void endFrame();

    float windowAspectRatio() const { return m_Window.aspectRatio(); }
    bool windowClosed() const { return m_Window.closed(); }

  private:
    void notifyWindowResized();
    void notifyWindowClosed();

    Window m_Window;
    bool m_WindowResized{};
    bool m_WindowClosed{};

    std::unique_ptr<Renderer> m_Renderer;
};

}  // namespace Melon
