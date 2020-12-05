#pragma once

#include <MelonCore/ArchetypeMask.h>
#include <MelonCore/SystemBase.h>
#include <MelonFrontend/MeshBuffer.h>
#include <MelonFrontend/RenderMesh.h>

#include <unordered_map>
#include <vector>

namespace MelonFrontend {

class RenderSystem : public MelonCore::SystemBase {
  public:
    RenderSystem(unsigned int const& width, unsigned int const& height);
    virtual ~RenderSystem();

  protected:
    virtual void onEnter() final;
    virtual void onUpdate() final;
    virtual void onExit() final;

  private:
    MelonCore::EntityFilter _createdRenderMeshEntityFilter;
    MelonCore::EntityFilter _renderMeshEntityFilter;
    MelonCore::EntityFilter _destroyedRenderMeshEntityFilter;
    unsigned int _translationComponentId;
    unsigned int _rotationComponentId;
    unsigned int _scaleComponentId;
    unsigned int _renderMeshComponentId;
    unsigned int _manualRenderMeshComponentId;

    unsigned int _currentWidth;
    unsigned int _currentHeight;

    std::unordered_map<unsigned int, unsigned int> _renderMeshReferenceCountMap;
    std::unordered_map<unsigned int, MeshBuffer> _meshBufferMap;
};

}  // namespace MelonFrontend
