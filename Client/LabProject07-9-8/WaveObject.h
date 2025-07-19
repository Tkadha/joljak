#include "Object.h"
#include "Waves.h"

class CWavesObject : public CGameObject
{
public:
    CWavesObject(int nMaterials, CGameFramework* pGameFramework);
    CWavesObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);
    virtual ~CWavesObject();

    // CGameObject의 가상 함수들을 오버라이드하여 WavesObject에 맞게 재정의합니다.
    virtual void Animate(float fTimeElapsed) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL) override;
    virtual void ReleaseUploadBuffers() override;

    void SetDisplacementMap(D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle) { m_d3dDisplacementMapGpuHandle = gpuHandle; }

private:
    // Waves 시뮬레이션을 위한 객체
    std::unique_ptr<Waves> m_pWaves;

    // 동적으로 업데이트될 정점 버퍼와 고정된 인덱스 버퍼
    ID3D12Resource* m_pd3dVertexBuffer;
    ID3D12Resource* m_pd3dIndexBuffer;
    ID3D12Resource* m_pd3dIndexUploadBuffer; // 인덱스 버퍼 생성을 위한 임시 업로드 버퍼

    D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;
    D3D12_INDEX_BUFFER_VIEW  m_d3dIndexBufferView;

    // 인덱스 데이터를 저장할 벡터
    std::vector<UINT> m_vIndices;

    D3D12_GPU_DESCRIPTOR_HANDLE m_d3dDisplacementMapGpuHandle = { 0 };
};