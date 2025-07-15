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


// Object.cpp ���� ���ϴܿ� �߰�
CWavesObject::CWavesObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework)
    : CGameObject(1, pGameFramework)
{
    // 1. Waves �ùķ��̼� ��ü�� �����մϴ�.
    // ����: (��, ��, �׸��� ����, �ð� ����, �ӵ�, ����)
    m_pWaves = std::make_unique<Waves>(400, 400, 3.8f, 0.03f, 5.0f, 0.2f);

    // 2. ���� �޽ø� �׸� �ε��� �����͸� �����մϴ�.
    // �� �����ʹ� �ѹ� �����Ǹ� ������ �ʽ��ϴ�.
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

    // 3. ���� ���� ���۸� �����մϴ�.
    // CPU���� �� ������ �����ؾ� �ϹǷ� D3D12_HEAP_TYPE_UPLOAD ���� �����մϴ�.
    UINT nVertexCount = m_pWaves->VertexCount();
    UINT nVertexSize = sizeof(Vertex);
    m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, nullptr, nVertexSize * nVertexCount, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr);
    m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
    m_d3dVertexBufferView.StrideInBytes = nVertexSize;
    m_d3dVertexBufferView.SizeInBytes = nVertexSize * nVertexCount;

    // 4. ������ �ε��� ���۸� �����մϴ�.
    // GPU�� �����ϹǷ� D3D12_HEAP_TYPE_DEFAULT ���� �����մϴ�.
    m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_vIndices.data(), sizeof(UINT) * m_vIndices.size(), D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_pd3dIndexUploadBuffer);
    m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
    m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_vIndices.size();
}

CWavesObject::~CWavesObject()
{
    // �Ҹ��ڿ��� ���ҽ��� �����մϴ�.
    if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
    if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
    if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release(); // �ӽ� ���ε� ���۵� ���� ���� ����
}

void CWavesObject::ReleaseUploadBuffers()
{
    // CGameObject�� ���� �Լ�. ���⼭�� �ε��� ���ε� ���۸� �����մϴ�.
    if (m_pd3dIndexUploadBuffer)
    {
        m_pd3dIndexUploadBuffer->Release();
        m_pd3dIndexUploadBuffer = nullptr;
    }
}

void CWavesObject::Animate(float fTimeElapsed)
{
    // 1. Waves �ùķ��̼� ������Ʈ
    if (m_pWaves)
    {
        // 0.25�ʸ��� ������ ��ġ�� ������ �����մϴ�.
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

    // 2. �ùķ��̼� ����� �������� ���� ������ ������ ������Ʈ�մϴ�.
    Vertex* pVertices = nullptr;
    // Map�� ���� CPU���� GPU �޸𸮿� ������ �� �ִ� �����͸� ����ϴ�.
    m_pd3dVertexBuffer->Map(0, nullptr, (void**)&pVertices);

    for (int i = 0; i < m_pWaves->VertexCount(); ++i)
    {
        pVertices[i].pos = m_pWaves->Position(i);
        pVertices[i].normal = m_pWaves->Normal(i);
        // �ؽ�ó ��ǥ�� ���� ��ǥ�踦 �״�� ����ϰų� �ʿ信 �°� �����մϴ�.
        pVertices[i].texc = { (m_pWaves->Position(i).x / m_pWaves->Width()) + 0.5f, (m_pWaves->Position(i).z / m_pWaves->Depth()) - 0.5f };
    }
    // ������Ʈ�� ������ Unmap�� ȣ���մϴ�.
    m_pd3dVertexBuffer->Unmap(0, nullptr);
}

void CWavesObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    // CGameObject::Render�� ������� �ϵ�, �޽ø� ���� �׸��� ������� �����մϴ�.
    CScene* pScene = m_pGameFramework ? m_pGameFramework->GetScene() : nullptr;
    if (!pScene) return;

    if (!pCamera) return;

    CMaterial* pMaterial = GetMaterial(0);
    if (pMaterial && pMaterial->m_pShader)
    {

        // ���������� ���� ����(���̴�, ��Ʈ �ñ״�ó ��)
        m_pGameFramework->GetScene()->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

        cbGameObjectInfo gameObjectInfo;

        XMFLOAT4X4 xmf4x4World;
        XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
        pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0); 



        // 2. ���� ī�޶��� ��� ���۸� ��Ʈ �Ķ���� 0�� ���Կ� ��������� ���ε��մϴ�.
        //    (�׸��� �н����� ������ ���� ī�޶� ���¸� ����ϴ�.)
        pd3dCommandList->SetGraphicsRootConstantBufferView(0, pCamera->GetCameraConstantBuffer()->GetGPUVirtualAddress());

        // 3. ���� ������ ��Ʈ �Ķ���� 2�� ���Կ� ���ε��մϴ�.
        ID3D12Resource* pLightBuffer = pScene->GetLightsConstantBuffer();
        if (pLightBuffer) {
            pd3dCommandList->SetGraphicsRootConstantBufferView(2, pLightBuffer->GetGPUVirtualAddress());
        }

        // 4. �׸��� ���� ��Ʈ �Ķ���� 4�� ���Կ� ���ε��մϴ�.
        pd3dCommandList->SetGraphicsRootDescriptorTable(4, pScene->GetShadowMapSrv());

        // ��ȯ ��� �� ���� ������Ʈ
        UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
        pMaterial->m_pShader->UpdateShaderVariables(pd3dCommandList);

        // ���� ���ۿ� �ε��� ���۸� ���������ο� ���ε��մϴ�.
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_d3dVertexBufferView);
        pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);

        // �ε��� ������ ������ ����ŭ ��ο� ���� ȣ���մϴ�.
        pd3dCommandList->DrawIndexedInstanced(m_vIndices.size(), 1, 0, 0, 0);
    }
}