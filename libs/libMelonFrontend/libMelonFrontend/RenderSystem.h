#pragma once

#include <libMelonCore/ArchetypeMask.h>
#include <libMelonCore/SystemBase.h>
#include <libMelonFrontend/Engine.h>
#include <libMelonFrontend/MeshBuffer.h>
#include <libMelonFrontend/RenderMesh.h>

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
    unsigned int m_TranslationComponentId;
    unsigned int m_RotationComponentId;
    unsigned int m_ScaleComponentId;
    unsigned int m_RenderMeshComponentId;
    unsigned int m_ManualRenderMeshComponentId;

    unsigned int m_CurrentWidth;
    unsigned int m_CurrentHeight;

    std::unordered_map<unsigned int, unsigned int> m_RenderMeshReferenceCountMap;
    std::unordered_map<unsigned int, MeshBuffer> m_MeshBufferMap;
};

}  // namespace Melon
