#include <MelonFrontend/Engine.h>

namespace MelonFrontend {

void Engine::initialize(char const* title, unsigned int const& width, unsigned int const& height) {
    _window.initialize(title, width, height);

    _renderer = std::make_unique<Renderer>();
    _renderer->initialize(&_window);
}

void Engine::terminate() {
    _renderer->terminate();

    _window.terminate();
}

void Engine::beginFrame() {
    _window.pollEvents();
    _renderer->beginFrame();
}

void Engine::beginBatches() { _renderer->beginBatches(); }

MeshBuffer Engine::createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices) {
    return _renderer->createMeshBuffer(vertices, indices);
}

void Engine::destroyMeshBuffer(MeshBuffer const& meshBuffer) {
    _renderer->destroyMeshBuffer(meshBuffer);
}

void Engine::addBatch(std::vector<glm::mat4> const& models, MeshBuffer const& meshBuffer) { _renderer->addBatch(models, meshBuffer); }

void Engine::endBatches() { _renderer->endBatches(); }

void Engine::renderFrame(/* TODO: Implement the Camera */ glm::mat4 const& vp) { _renderer->renderFrame(vp); }

void Engine::endFrame() { _renderer->endFrame(); }

Engine* Engine::instance() {
    static Engine sInstance;
    return &sInstance;
}

Engine::Engine() {}

void Engine::notifyWindowResized() {
    _windowResized = true;
}

void Engine::notifyWindowClosed() {
    _windowClosed = true;
}

}  // namespace MelonFrontend
