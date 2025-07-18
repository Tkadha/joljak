#pragma once

#include "Scene.h"
#include "d3dx12.h"
#include "ResourceManager.h"


class CConstructionSystem
{
public:
    void Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pGameFramework, CScene* scene);

   
    void EnterBuildMode(const std::string& prefabName, const CCamera* pCamera);
    void ExitBuildMode();
    void UpdatePreviewPosition(const CCamera* pCamera);
    void ConfirmPlacement();

    bool IsBuildMode() const { return m_bBuildMode; }

private:
    bool m_bBuildMode = false;
    CGameObject* m_pPreviewObject = nullptr; 

    
    ID3D12Device* m_pd3dDevice = nullptr;
    ID3D12GraphicsCommandList* m_pd3dCommandList = nullptr;
    CGameFramework* m_pGameFramework = nullptr;
    CScene* m_pScene = nullptr;
};
