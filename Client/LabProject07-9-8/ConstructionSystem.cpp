#include "ConstructionSystem.h"
#include "GameFramework.h" 
#include "Object.h" 

void CConstructionSystem::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pGameFramework, CScene* scene)
{
    m_pd3dDevice = device;
    m_pd3dCommandList = cmdList;
    m_pGameFramework = pGameFramework;
    m_pScene = scene;
}

void CConstructionSystem::EnterBuildMode(const std::string& prefabName, const CCamera* pCamera)
{
    
    if (m_pPreviewObject) ExitBuildMode();

    m_bBuildMode = true;

   
    ResourceManager* pResourceManager = m_pGameFramework->GetResourceManager();
    std::shared_ptr<CGameObject> pPrefab = pResourceManager->GetPrefab(prefabName);

    if (!pPrefab) {
        OutputDebugStringA(("[Build] ❌ Prefab not found in ResourceManager: " + prefabName + "\n").c_str());
        m_bBuildMode = false;
        return;
    }

   
    m_pPreviewObject = pPrefab->Clone();
    if (!m_pPreviewObject) {
        m_bBuildMode = false;
        return;
    }

   
    m_pPreviewObject->UpdateTransform(NULL);
    m_pPreviewObject->isRender = true;
    m_pScene->m_pPreviewPine = m_pPreviewObject; 

    if (pCamera) UpdatePreviewPosition(pCamera);
}


void CConstructionSystem::ExitBuildMode()
{
    if (m_pPreviewObject) {
        delete m_pPreviewObject; 
        m_pPreviewObject = nullptr;
    }
    if (m_pScene) {
        m_pScene->m_pPreviewPine = nullptr; 
    }
    m_bBuildMode = false;
}

void CConstructionSystem::UpdatePreviewPosition(const CCamera* pCamera)
{
    if (!m_bBuildMode || !m_pPreviewObject || !pCamera) return;

    
    XMFLOAT3 camPos = pCamera->GetPosition();
    XMFLOAT3 camLook = pCamera->GetLookVector();

    XMVECTOR vPos = XMLoadFloat3(&camPos);
    XMVECTOR vLook = XMLoadFloat3(&camLook);
    XMVECTOR vTargetPos = XMVectorAdd(vPos, XMVectorScale(vLook, 100.0f));

    XMFLOAT3 previewPos;
    XMStoreFloat3(&previewPos, vTargetPos);

    

    m_pPreviewObject->SetPosition(previewPos);
}

void CConstructionSystem::ConfirmPlacement()
{
    if (!m_bBuildMode || !m_pPreviewObject || !m_pScene) return;

   
    CGameObject* pInstalledObject = m_pPreviewObject->Clone();
    if (!pInstalledObject) return;

   
    pInstalledObject->UpdateTransform(NULL);
    m_pScene->m_vGameObjects.push_back(pInstalledObject);

    // (선택) 설치 후 바로 건설 모드를 종료하려면 아래 주석 해제
    // ExitBuildMode();
}