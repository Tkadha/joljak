#include "WaveObject.h"
#include "Scene.h"
#include "GameFramework.h"

// Object.cpp ���� ���ϴܿ� �߰�
CWavesObject::CWavesObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework)
    : CGameObject(1, pGameFramework)
{
    // 1. Waves �ùķ��̼� ��ü�� �����մϴ�.
    // ����: (��, ��, �׸��� ����, �ð� ����, �ӵ�, ����)
    m_pWaves = std::make_unique<Waves>(100, 100, 2.8f, 0.03f, 3.25f, 0.4f);

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
            float r = MathHelper::RandF(1.0f, 2.0f);
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
    if (!pCamera) return;

    CMaterial* pMaterial = GetMaterial(0);
    if (pMaterial && pMaterial->m_pShader)
    {
        // ���������� ���� ����(���̴�, ��Ʈ �ñ״�ó ��)
        m_pGameFramework->GetScene()->SetGraphicsState(pd3dCommandList, pMaterial->m_pShader);

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