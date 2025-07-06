#include "ConstructionSystem.h"

void CConstructionSystem::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pGameFramework, CScene* scene)
{
    m_pd3dDevice = device;
    m_pd3dCommandList = cmdList;
    m_pGameFramework = pGameFramework;
    m_pScene = scene;
}

void CConstructionSystem::EnterBuildMode(const CCamera* pCamera)
{
    if (m_bBuildMode) return; 
    m_pPreviewObject = m_pScene->m_pPreviewPine;
    m_pPreviewObject->SetPosition(previewPos);
    m_pPreviewObject->isRender = true;
  
    if (pCamera)
        UpdatePreviewPosition(pCamera);

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

    
    XMFLOAT3 camPos = pCamera->GetPosition();
    XMFLOAT3 camLook = pCamera->GetLookVector();

    XMVECTOR vCamPos = XMLoadFloat3(&camPos);
    XMVECTOR vCamLook = XMLoadFloat3(&camLook);
    XMVECTOR vTarget = XMVectorAdd(vCamPos, XMVectorScale(vCamLook, 100.f));


    XMStoreFloat3(&previewPos, vTarget);
    //previewPos.y += 40.f;
    m_xmf3PreviewPosition = previewPos;

    m_pPreviewObject->SetPosition(previewPos);
    //m_pScene->octree.update(m_pPreviewObject->m_treecount, previewPos);

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
    CGameObject* installedObject = new CConstructionObject(m_pd3dDevice, m_pd3dCommandList, m_pGameFramework);
    installedObject->SetPosition(m_pPreviewObject->GetPosition());
    installedObject->SetOBB(1.f, 1.f, 1.f, XMFLOAT3(0.f, 0.f, 0.f));
    installedObject->InitializeOBBResources(m_pd3dDevice, m_pd3dCommandList);
    installedObject->isRender = true;
    m_pScene->m_vGameObjects.emplace_back(installedObject);

    // 새 프리뷰 오브젝트도 같은 방식으로 생성
    CGameObject* newPreview = new CConstructionObject(m_pd3dDevice, m_pd3dCommandList, m_pGameFramework);
    newPreview->SetOBB(1.f, 1.f, 1.f, XMFLOAT3(0.f, 0.f, 0.f));
    newPreview->InitializeOBBResources(m_pd3dDevice, m_pd3dCommandList);
    newPreview->isRender = true;

    m_pPreviewObject = newPreview;
    m_pScene->m_pPreviewPine = newPreview;
    

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