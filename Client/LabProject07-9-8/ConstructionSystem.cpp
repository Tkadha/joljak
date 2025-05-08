#include "ConstructionSystem.h"

void CConstructionSystem::Init(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pGameFramework)
{
    m_pd3dDevice = device;
    m_pd3dCommandList = cmdList;
    m_pGameFramework = pGameFramework;
}

void CConstructionSystem::EnterBuildMode()
{
    if (m_bBuildMode) return; // 이미 진입했으면 무시

    // 더미 오브젝트 생성
    m_pPreviewObject = new CGameObject();
    m_pPreviewObject->SetPosition(XMFLOAT3(0.f, 0.f, 0.f));
    m_pPreviewObject->SetScale(100.f, 100.f, 100.f); // 보기 좋은 크기
    m_pPreviewObject->SetColor(XMFLOAT4(1.f, 0.f, 0.f, 1.f)); // 눈에 띄는 색상 (빨강)

    m_bBuildMode = true;
}


void CConstructionSystem::ExitBuildMode()
{
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
    XMVECTOR vTarget = XMVectorAdd(vCamPos, XMVectorScale(vCamLook, 500.f));

    XMFLOAT3 previewPos;
    XMStoreFloat3(&previewPos, vTarget);
    m_xmf3PreviewPosition = previewPos;

    m_pPreviewObject->SetPosition(previewPos);
    
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