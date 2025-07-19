#include "Object.h"
#include "Waves.h"

class CWavesObject : public CGameObject
{
public:
    CWavesObject(int nMaterials, CGameFramework* pGameFramework);
    CWavesObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
    virtual ~CWavesObject();

    // CGameObject�� ���� �Լ����� �������̵��Ͽ� WavesObject�� �°� �������մϴ�.
    virtual void Animate(float fTimeElapsed) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL) override;
    virtual void ReleaseUploadBuffers() override;

    void SetDisplacementMap(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { m_d3dDisplacementMapGpuHandle = gpuHandle; }

private:
    // Waves �ùķ��̼��� ���� ��ü
    std::unique_ptr<Waves> m_pWaves;

    // �������� ������Ʈ�� ���� ���ۿ� ������ �ε��� ����
    ID3D12Resource* m_pd3dVertexBuffer;
    ID3D12Resource* m_pd3dIndexBuffer;
    ID3D12Resource* m_pd3dIndexUploadBuffer; // �ε��� ���� ������ ���� �ӽ� ���ε� ����

    D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;
    D3D12_INDEX_BUFFER_VIEW  m_d3dIndexBufferView;

    // �ε��� �����͸� ������ ����
    std::vector<UINT> m_vIndices;

    D3D12_GPU_DESCRIPTOR_HANDLE m_d3dDisplacementMapGpuHandle = { 0 };
};