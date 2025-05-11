#include "ConstructionSystem.h"

void CConstructionSystem::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pGameFramework, CScene* scene)
{
    m_pd3dDevice = device;
    m_pd3dCommandList = cmdList;
    m_pGameFramework = pGameFramework;
    m_pScene = scene;
}

void CConstructionSystem::EnterBuildMode()
{
    if (m_bBuildMode) return; // 이미 진입했으면 무시
    m_pPreviewObject = m_pScene->m_pPreviewPine;
    m_pPreviewObject->SetPosition(previewPos);
    m_pPreviewObject->isRender = true;
    // 더미 오브젝트 생성
   

    m_bBuildMode = true;
}


void CConstructionSystem::ExitBuildMode()
{
    if (m_pPreviewObject)
        m_pPreviewObject->isRender = false;
    m_bBuildMode = false;
}

void CConstructionSystem::UpdatePreviewPosition(const CCamera* pCamera)
{
    
    if (!m_bBuildMode || !m_pPreviewObject) return;

    // 카메라 기준 위치 계산 (앞으로 500만큼)
    XMFLOAT3 camPos = pCamera->GetPosition();
    XMFLOAT3 camLook = pCamera->GetLookVector();

    XMVECTOR vCamPos = XMLoadFloat3(&camPos);
    XMVECTOR vCamLook = XMLoadFloat3(&camLook);
    XMVECTOR vTarget = XMVectorAdd(vCamPos, XMVectorScale(vCamLook, 300.f));


    XMStoreFloat3(&previewPos, vTarget);
    previewPos.y += 40.f;
    m_xmf3PreviewPosition = previewPos;

    m_pPreviewObject->SetPosition(previewPos);
    m_pScene->octree.update(m_pPreviewObject->m_treecount, previewPos);

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

void CConstructionSystem::ConfirmPlacement()
{
    m_bBuildMode = false;


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