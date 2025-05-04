#include "ConstructionSystem.h"

void CConstructionSystem::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pGameFramework)
{
    m_pd3dDevice = device;
    m_pd3dCommandList = cmdList;
    m_pGameFramework = pGameFramework;
}

void CConstructionSystem::EnterBuildMode()
{
    if (m_pPreviewObject)
    {
        delete m_pPreviewObject;
        m_pPreviewObject = nullptr;
    }

    m_pPreviewObject = CGameObject::LoadGeometryFromFile(
        m_pd3dDevice,
        m_pd3dCommandList,
        "Model/FAE_Pine_A_LOD0.bin", // 우선 소나무로 테스트
        m_pGameFramework
    );

    m_bBuildMode = true;
}


void CConstructionSystem::ExitBuildMode()
{
    m_bBuildMode = false;
}

void CConstructionSystem::UpdatePreview(const XMFLOAT3& playerPos, const XMFLOAT3& forward)
{
    if (!m_bBuildMode || !m_pPreviewObject) return;

    XMFLOAT3 previewPos = {
        playerPos.x + forward.x * 300.f,
        playerPos.y,
        playerPos.z + forward.z * 300.f
    };

    m_pPreviewObject->SetPosition(previewPos);
}

void CConstructionSystem::ConfirmPlacement(std::vector<CGameObject*>& sceneObjects)
{
    if (!m_pPreviewObject) return;

    auto* newWall = new CPineObject(m_pd3dDevice, m_pd3dCommandList, m_pGameFramework);
    newWall->SetPosition(m_pPreviewObject->GetPosition());
    sceneObjects.push_back(newWall);
}

void CConstructionSystem::RenderPreview(ID3D12GraphicsCommandList* cmdList, CCamera* camera)
{
    if (m_bBuildMode && m_pPreviewObject)
        m_pPreviewObject->Render(cmdList, camera);
}
void CConstructionSystem::SetSelectedBuilding(const std::string& name)
{
    m_sSelectedBuilding = name;
}