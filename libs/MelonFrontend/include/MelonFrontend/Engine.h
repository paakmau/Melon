#pragma once

#include <MelonFrontend/MeshBuffer.h>
#include <MelonFrontend/RenderBatch.h>
#include <MelonFrontend/Renderer.h>
#include <MelonFrontend/SwapChain.h>
#include <MelonFrontend/Window.h>

#include <cstddef>
#include <glm/mat4x4.hpp>
#include <memory>
#include <vector>

namespace MelonFrontend {

class Engine {
   public:
    void initialize(char const* title, unsigned int const& width, unsigned int const& height);
    void terminate();

    void beginFrame();
    MeshBuffer createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    void destroyMeshBuffer(MeshBuffer const& meshBuffer);
    void beginBatches();
    void addBatch(std::vector<glm::mat4> const& models, MeshBuffer const& meshBuffer);
    void endBatches();
    void renderFrame(glm::mat4 const& vp);
    void endFrame();

    static Engine* instance();

    float windowAspectRatio() const { return m_Window.aspectRatio(); }
    bool windowClosed() const { return m_Window.closed(); }

   private:
    Engine();

    void notifyWindowResized();
    void notifyWindowClosed();

    Window m_Window;
    bool m_WindowResized{};
    bool m_WindowClosed{};

    std::unique_ptr<Renderer> m_Renderer;
};

}  // namespace MelonFrontend
