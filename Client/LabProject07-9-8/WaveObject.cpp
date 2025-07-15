#include "WaveObject.h"
#include "Scene.h"
#include "GameFramework.h"


struct cbGameObjectInfo {
    XMFLOAT4X4    gmtxGameObject;     // 16 DWORDS
    struct MaterialInfoCpp {
        XMFLOAT4   AmbientColor;  // 4
        XMFLOAT4   DiffuseColor;  // 4
        XMFLOAT4   SpecularColor; // 4
        XMFLOAT4   EmissiveColor; // 4
        float      Glossiness;        // 1
        float      Smoothness;        // 1
        float      SpecularHighlight; // 1
        float      Metallic;          // 1
        float      GlossyReflection;  // 1
        XMFLOAT3   Padding;           // 3 => MaterialInfoCpp = 24 DWORDS
    } gMaterialInfo;
    UINT          gnTexturesMask;     // 1 DWORD

};


// Object.cpp 파일 최하단에 추가
CWavesObject::CWavesObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework)
    : CGameObject(1, pGameFramework)
{
    // 1. Waves 시뮬레이션 객체를 생성합니다.
    // 인자: (행, 열, 그리드 간격, 시간 간격, 속도, 감쇠)
    m_pWaves = std::make_unique<Waves>(400, 400, 3.8f, 0.03f, 5.0f, 0.2f);

    // 2. 물결 메시를 그릴 인덱스 데이터를 생성합니다.
    // 이 데이터는 한번 생성되면 변하지 않습니다.
    m_vIndices.resize(m_pWaves->TriangleCount() * 3);
    const int m = m_pWaves->RowCount();
    const int n = m_pWaves->ColumnCount();
    int k = 0;
    for (int i = 0; i < m - 1; ++i)
    {
        for (int j = 0; j < n - 1; ++j)
        {
            m_vIndices[k] = i * n + j;
            m_vIndices[k + 1] = i * n + j + 1;
            m_vIndices[k + 2] = (i + 1) * n + j;

            m_vIndices[k + 3] = (i + 1) * n + j;
            m_vIndices[k + 4] = i * n + j + 1;
            m_vIndices[k + 5] = (i + 1) * n + j + 1;
            k += 6;
        }
    }

    // 3. 동적 정점 버퍼를 생성합니다.
    // CPU에서 매 프레임 접근해야 하므로 D3D12_HEAP_TYPE_UPLOAD 힙에 생성합니다.
    UINT nVertexCount = m_pWaves->VertexCount();
    UINT nVertexSize = sizeof(Vertex);
    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, nullptr, nVertexSize * nVertexCount, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes = nVertexSize;
    m_d3dVertexBufferView.SizeInBytes = nVertexSize * nVertexCount;

    // 4. 고정된 인덱스 버퍼를 생성합니다.
    // GPU만 접근하므로 D3D12_HEAP_TYPE_DEFAULT 힙에 생성합니다.
    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_vIndices.data(), sizeof(UINT) * m_vIndices.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_vIndices.size();
}

CWavesObject::~CWavesObject()
{
    // 소멸자에서 리소스를 해제합니다.
    if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
    if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
    if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release(); // 임시 업로드 버퍼도 잊지 말고 해제
}

void CWavesObject::ReleaseUploadBuffers()
{
    // CGameObject의 가상 함수. 여기서는 인덱스 업로드 버퍼를 해제합니다.
    if (m_pd3dIndexUploadBuffer)
    {
        m_pd3dIndexUploadBuffer->Release();
        m_pd3dIndexUploadBuffer = nullptr;
    }
}

void CWavesObject::Animate(float fTimeElapsed)
{
    // 1. Waves 시뮬레이션 업데이트
    if (m_pWaves)
    {
        // 0.25초마다 무작위 위치에 물결을 생성합니다.
        static float t_base = 0.0f;
        //CGameFramework* m_pGameFramework =  m_pScene->GetGameFramework();
        if ((m_pGameFramework->m_GameTimer.GetTotalTime() - t_base) >= 0.25f)
        {
            t_base += 0.25f;
            int i = 5 + rand() % (m_pWaves->RowCount() - 10);
            int j = 5 + rand() % (m_pWaves->ColumnCount() - 10);
            float r = MathHelper::RandF(0.5f, 1.0f);
            m_pWaves->Disturb(i, j, r);
        }
        m_pWaves->Update(fTimeElapsed);
    }

    // 2. 시뮬레이션 결과를 바탕으로 정점 버퍼의 내용을 업데이트합니다.
    Vertex* pVertices = nullptr;
    // Map을 통해 CPU에서 GPU 메모리에 접근할 수 있는 포인터를 얻습니다.
    m_pd3dVertexBuffer->Map(0, nullptr, (void**)&pVertices);

    for (int i = 0; i < m_pWaves->VertexCount(); ++i)
    {
        pVertices[i].pos = m_pWaves->Position(i);
        pVertices[i].normal = m_pWaves->Normal(i);
        // 텍스처 좌표는 월드 좌표계를 그대로 사용하거나 필요에 맞게 수정합니다.
        pVertices[i].texc = { (m_pWaves->Position(i).x / m_pWaves->Width()) + 0.5f, (m_pWaves->Position(i).z / m_pWaves->Depth()) - 0.5f };
    }
    // 업데이트가 끝나면 Unmap을 호출합니다.
    m_pd3dVertexBuffer->Unmap(0, nullptr);
}

void CWavesObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    // CGameObject::Render를 기반으로 하되, 메시를 직접 그리는 방식으로 수정합니다.
    CScene* pScene = m_pGameFramework ? m_pGameFramework->GetScene() : nullptr;
    if (!pScene) return;

    if (!pCamera) return;

    CMaterial* pMaterial = GetMaterial(0);
    if (pMaterial && pMaterial->m_pShader)
    {

        // 파이프라인 상태 설정(셰이더, 루트 시그니처 등)
        m_pGameFramework->GetScene()->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

        cbGameObjectInfo gameObjectInfo;

        XMFLOAT4X4 xmf4x4World;
        XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
        pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0); 



        // 2. 메인 카메라의 상수 버퍼를 루트 파라미터 0번 슬롯에 명시적으로 바인딩합니다.
        //    (그림자 패스에서 설정된 빛의 카메라 상태를 덮어씁니다.)
        pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());

        // 3. 조명 정보를 루트 파라미터 2번 슬롯에 바인딩합니다.
        ID3D12Resource* pLightBuffer = pScene->GetLightsConstantBuffer();
        if (pLightBuffer) {
            pd3dCommandList->SetGraphicsRootConstantBufferView(2, pLightBuffer->GetGPUVirtualAddress());
        }

        // 4. 그림자 맵을 루트 파라미터 4번 슬롯에 바인딩합니다.
        pd3dCommandList->SetGraphicsRootDescriptorTable(4, pScene->GetShadowMapSrv());

        // 변환 행렬 및 재질 업데이트
        UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
        pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

        // 정점 버퍼와 인덱스 버퍼를 파이프라인에 바인딩합니다.
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView);
        pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);

        // 인덱스 버퍼의 데이터 수만큼 드로우 콜을 호출합니다.
        pd3dCommandList->DrawIndexedInstanced(m_vIndices.size(), 1, 0, 0, 0);
    }
}