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

void CConstructionSystem::RotatePreviewObject(float fYaw)
{
    if (!m_bBuildMode || !m_pPreviewObject) return;

    // Y축(Up Vector)을 기준으로 회전
    m_pPreviewObject->Rotate(0.0f, fYaw, 0.0f);
}

CGameObject* CConstructionSystem::ConfirmPlacement()
{
    if (!m_bBuildMode || !m_pPreviewObject) return nullptr;

    // 1. 복제의 원본이 될 현재 프리뷰 오브젝트를 기억합니다.
    CGameObject* pObjectToClone = m_pPreviewObject;

    // 2. 이제 프리뷰는 역할을 다했으므로 화면에서 숨기고 비활성화합니다.
    m_pPreviewObject->isRender = false;
    m_pPreviewObject = nullptr;

    // 3. GameFramework가 복제할 수 있도록 원본 오브젝트를 반환합니다.
    return pObjectToClone;
}