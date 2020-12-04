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
    void initialize(const char* title, const unsigned int& width, const unsigned int& height);
    void terminate();

    void beginFrame();
    MeshBuffer createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices);
    void destroyMeshBuffer(const MeshBuffer& meshBuffer);
    void beginBatches();
    void addBatch(const std::vector<glm::mat4>& models, const MeshBuffer& meshBuffer);
    void endBatches();
    void renderFrame(const glm::mat4& vp);
    void endFrame();

    static Engine* instance();

    float windowAspectRatio() const { return _window.aspectRatio(); }
    bool windowClosed() const { return _window.closed(); }

   private:
    Engine();

    void notifyWindowResized();
    void notifyWindowClosed();

    Window _window;
    bool _windowResized{};
    bool _windowClosed{};

    std::unique_ptr<Renderer> _renderer;
};

}  // namespace MelonFrontend
