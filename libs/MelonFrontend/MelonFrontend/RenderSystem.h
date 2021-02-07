#pragma once

#include <MelonCore/ArchetypeMask.h>
#include <MelonCore/SystemBase.h>
#include <MelonFrontend/Engine.h>
#include <MelonFrontend/InputEvents.h>
#include <MelonFrontend/MeshBuffer.h>
#include <MelonFrontend/RenderMesh.h>

#include <unordered_map>
#include <vector>

namespace Melon {

class RenderSystem : public SystemBase {
  public:
    RenderSystem(const unsigned int& width, const unsigned int& height);
    virtual ~RenderSystem();

  protected:
    virtual void onEnter() final;
    virtual void onUpdate() final;
    virtual void onExit() final;

  private:
    Engine m_Engine;

    EntityFilter m_CreatedRenderMeshEntityFilter;
    EntityFilter m_RenderMeshEntityFilter;
    EntityFilter m_DestroyedRenderMeshEntityFilter;
    EntityFilter m_CameraEntityFilter;
    EntityFilter m_LightEntityFilter;

    unsigned int m_TranslationComponentId;
    unsigned int m_RotationComponentId;
    unsigned int m_ScaleComponentId;
    unsigned int m_PerspectiveProjectionComponentId;
    unsigned int m_RenderMeshComponentId;
    unsigned int m_ManualRenderMeshComponentId;
    unsigned int m_LightComponentId;

    unsigned int m_KeyDownEventId;
    unsigned int m_KeyUpEventId;
    unsigned int m_MouseButtonDownEventId;
    unsigned int m_MouseButtonUpEventId;
    unsigned int m_MouseScrollEventId;

    unsigned int m_CurrentWidth;
    unsigned int m_CurrentHeight;

    std::unordered_map<unsigned int, unsigned int> m_RenderMeshReferenceCountMap;
    std::unordered_map<unsigned int, MeshBuffer> m_MeshBufferMap;
};

}  // namespace Melon
