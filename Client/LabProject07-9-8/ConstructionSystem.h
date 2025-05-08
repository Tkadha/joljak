#pragma once

#include "Scene.h"
#include "d3dx12.h"
#include "ResourceManager.h"


class CConstructionSystem
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pGameFramework);
    void EnterBuildMode();
    void ExitBuildMode();
    void UpdatePreview(const XMFLOAT3& playerPos, const XMFLOAT3& forward);
    void ConfirmPlacement(std::vector<CGameObject*>& sceneObjects);
    void SetSelectedBuilding(const std::string& name);
    void UpdatePreviewPosition(const CCamera* pCamera);
    XMFLOAT3 GetPreviewPosition() const { return m_xmf3PreviewPosition; }

    void RenderPreview(ID3D12GraphicsCommandList* cmdList, CCamera* camera);

    bool IsBuildMode() const { return m_bBuildMode; }

private:
    bool m_bBuildMode = false;
    CGameObject* m_pPreviewObject = nullptr;
    std::string m_sSelectedBuilding = "pine"; // 기본값은 pine

    ID3D12Device* m_pd3dDevice = nullptr;
    ID3D12GraphicsCommandList* m_pd3dCommandList = nullptr;
    //ID3D12RootSignature* m_pd3dRootSignature = nullptr;
    //ResourceManager* m_pResourceManager = nullptr;

    CGameFramework* m_pGameFramework = nullptr;
    XMFLOAT3 m_xmf3PreviewPosition;
};
