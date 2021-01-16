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
    void initialize(MelonTask::TaskManager* taskManager, const char* title, const unsigned int& width, const unsigned int& height);
    void terminate();

    void beginFrame();
    MeshBuffer createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    void destroyMeshBuffer(const MeshBuffer& meshBuffer);
    void beginBatches();
    void addBatch(std::vector<glm::mat4> const& models, const MeshBuffer& meshBuffer);
    void endBatches();
    void renderFrame(const glm::mat4& projection, const glm::vec3& cameraTranslation, const glm::quat& cameraRotation);
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
