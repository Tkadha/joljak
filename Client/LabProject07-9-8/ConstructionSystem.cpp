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
    
    if (m_pPreviewObject) {
        m_pPreviewObject->isRender = false;
    }

    m_bBuildMode = true;

    // Scene에 미리 생성된 프리뷰 오브젝트를 이름으로 찾아온다.
    if (m_pScene && m_pScene->m_mapBuildPrefabs.count(prefabName)) {
        m_pPreviewObject = m_pScene->m_mapBuildPrefabs[prefabName];
        m_pPreviewObject->isRender = true; // 찾아온 오브젝트를 보이게 만든다.
    }
    else {
        m_pPreviewObject = nullptr; // 못 찾았으면 null 처리
        m_bBuildMode = false;
        return;
    }

    if (pCamera) UpdatePreviewPosition(pCamera);
}


void CConstructionSystem::ExitBuildMode()
{
    m_bBuildMode = false;
    // 활성화된 프리뷰 오브젝트가 있다면 다시 숨긴다.
    if (m_pPreviewObject) {
        m_pPreviewObject->isRender = false;
        m_pPreviewObject = nullptr; // 현재 활성화된 프리뷰가 없음을 표시
    }
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