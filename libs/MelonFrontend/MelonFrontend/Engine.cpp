#include <MelonFrontend/Engine.h>

namespace Melon {

void Engine::initialize(TaskManager* taskManager, const char* title, const unsigned int& width, const unsigned int& height) {
    m_Window.initialize(title, width, height);

    m_Renderer = std::make_unique<Renderer>();
    m_Renderer->initialize(taskManager, &m_Window);
}

void Engine::terminate() {
    m_Renderer->terminate();

    m_Window.terminate();
}

void Engine::beginFrame() {
    m_Window.pollEvents();
    m_Renderer->beginFrame();
}

void Engine::beginBatches() { m_Renderer->beginBatches(); }

MeshBuffer Engine::createMeshBuffer(std::vector<Vertex> vertices, std::vector<uint16_t> indices) {
    return m_Renderer->createMeshBuffer(vertices, indices);
}

void Engine::destroyMeshBuffer(const MeshBuffer& meshBuffer) {
    m_Renderer->destroyMeshBuffer(meshBuffer);
}

void Engine::addBatch(std::vector<glm::mat4> const& models, const MeshBuffer& meshBuffer) { m_Renderer->addBatch(models, meshBuffer); }

void Engine::endBatches() { m_Renderer->endBatches(); }

void Engine::renderFrame(const glm::mat4& projection, const glm::vec3& cameraTranslation, const glm::quat& cameraRotation, const glm::vec3& lightDirection) { m_Renderer->renderFrame(projection, cameraTranslation, cameraRotation, lightDirection); }

void Engine::endFrame() { m_Renderer->endFrame(); }

void Engine::notifyWindowResized() {
    m_WindowResized = true;
}

void Engine::notifyWindowClosed() {
    m_WindowClosed = true;
}

}  // namespace Melon
