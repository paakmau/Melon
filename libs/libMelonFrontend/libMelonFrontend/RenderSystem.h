#pragma once

#include <libMelonCore/ArchetypeMask.h>
#include <libMelonCore/SystemBase.h>
#include <libMelonFrontend/MeshBuffer.h>
#include <libMelonFrontend/RenderMesh.h>

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
    MelonCore::EntityFilter m_CreatedRenderMeshEntityFilter;
    MelonCore::EntityFilter m_RenderMeshEntityFilter;
    MelonCore::EntityFilter m_DestroyedRenderMeshEntityFilter;
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

}  // namespace MelonFrontend
