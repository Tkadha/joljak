//***************************************************************************************
// Waves.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Performs the calculations for the wave simulation.  After the simulation has been
// updated, the client must copy the current solution into vertex buffers for rendering.
// This class only does the calculations, it does not do any drawing.
//***************************************************************************************

#ifndef WAVES_H
#define WAVES_H

#include <vector>
#include <DirectXMath.h>
#include <d3d12.h>
#include <wrl.h>

class Waves
{
public:
    Waves(int m, int n, float dx, float dt, float speed, float damping);
    Waves(const Waves& rhs) = delete;
    Waves& operator=(const Waves& rhs) = delete;
    ~Waves();

    int RowCount()const;
    int ColumnCount()const;
    int VertexCount()const;
    int TriangleCount()const;
    float Width()const;
    float Depth()const;

    // 시뮬레이션 결과에 접근 (CPU 데이터)
    const DirectX::XMFLOAT3& Position(int i) const { return mCurrSolution[i]; }
    const DirectX::XMFLOAT3& Normal(int i) const { return mNormals[i]; }
    const DirectX::XMFLOAT3& TangentX(int i) const { return mTangentX[i]; } // 필요하다면

    // GPU 리소스 ID 또는 핸들 (ResourceManager가 관리)
    // 예시: 실제 ID 타입은 ResourceManager 설계에 따라 다름
    size_t GetPrevSolUAV() const { return mPrevSolUAVHandle; }
    size_t GetCurrSolUAV() const { return mCurrSolUAVHandle; }
    size_t GetNextSolUAV() const { return mNextSolUAVHandle; }

    // CurrSol은 렌더링 시 SRV로도 사용될 수 있음
    size_t GetCurrSolSRV() const { return mCurrSolSRVHandle; }

    void Update(float dt);
    void Disturb(int i, int j, float magnitude);

    // 초기화 (ResourceManager를 통해 GPU 리소스 생성 요청)
   // 이 함수는 ResourceManager와 Device 접근이 가능한 곳에서 호출되어야 함
   // bool InitResources(ID3D12Device* device, ResourceManager* resManager);
   // 또는 Scene/GameFramework에서 직접 리소스를 생성하고 핸들을 Waves 객체에 넘겨줄 수도 있음
    void SetResourceHandles(size_t prevSolUAV, size_t currSolUAV, size_t nextSolUAV, size_t currSolSRV);


    // Simulation constants we can precompute.
    float mK1 = 0.0f;
    float mK2 = 0.0f;
    float mK3 = 0.0f;
    float mTimeStep = 0.0f;     // public으로 변경 또는 getter 추가
    float mSpatialStep = 0.0f;  // public으로 변경 또는 getter 추가


private:
    int mNumRows = 0;
    int mNumCols = 0;

    int mVertexCount = 0;
    int mTriangleCount = 0;


    std::vector<DirectX::XMFLOAT3> mPrevSolution;
    std::vector<DirectX::XMFLOAT3> mCurrSolution;
    std::vector<DirectX::XMFLOAT3> mNormals;
    std::vector<DirectX::XMFLOAT3> mTangentX;


    size_t mPrevSolUAVHandle = -1;
    size_t mCurrSolUAVHandle = -1;
    size_t mNextSolUAVHandle = -1;
    size_t mCurrSolSRVHandle = -1; // CurrSol을 SRV로도 사용할 경우


public:
    // GPU 리소스 (UAV로 사용될 텍스처들)
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pPrevSolTexture;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pCurrSolTexture;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pNextSolTexture;

    // 각 리소스에 대한 디스크립터 핸들
    D3D12_CPU_DESCRIPTOR_HANDLE m_hPrevSolUavCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hPrevSolUavGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE m_hCurrSolUavCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hCurrSolUavGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE m_hNextSolUavCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hNextSolUavGPU;

    // m_pCurrSolTexture는 렌더링 시 SRV로도 사용됨
    D3D12_CPU_DESCRIPTOR_HANDLE m_hCurrSolSrvCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hCurrSolSrvGPU;

    // 핑퐁 로직을 위한 현재 상태의 핸들 및 리소스 반환 함수들 (구현 필요)
    ID3D12Resource* GetCurrentPrevSolTexture() const;
    ID3D12Resource* GetCurrentCurrSolTexture() const;
    ID3D12Resource* GetCurrentNextSolTexture() const; // 컴퓨트 셰이더의 출력 대상
    ID3D12Resource* GetCurrentCurrSolTextureForRendering() const; // 렌더링 시 SRV로 사용될 텍스처

    D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentPrevSolUAV_GPU() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentCurrSolUAV_GPU() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentNextSolUAV_GPU() const;
    // 컴퓨트 셰이더 루트 시그니처가 UAV 3개를 하나의 테이블로 받는다면, 그 시작 핸들
    D3D12_GPU_DESCRIPTOR_HANDLE GetUAVDescriptorTableStartGPU() const;


    void SwapSimBuffersAndDescriptors(); // 핑퐁을 위한 버퍼 및 핸들 교체 로직

private:
    // 핑퐁 로직을 위한 내부 인덱스 또는 상태 변수
    int m_nCurrentBufferIndex = 0; // 예시: 0, 1, 2 를 순환하며 Prev, Curr, Next 역할 지정
    // 또는 ComPtr<ID3D12Resource> 배열과 핸들 배열을 두고 인덱스로 접근4

private:
    ID3D12Device* m_pd3dDevice = nullptr;
    DXGI_FORMAT m_Format = DXGI_FORMAT_UNKNOWN;

public:
    void InitializeGpuResourcesAndHandles(
        ID3D12Device* device,
        DXGI_FORMAT format,
        Microsoft::WRL::ComPtr<ID3D12Resource> pPrevTex,
        Microsoft::WRL::ComPtr<ID3D12Resource> pCurrTex,
        Microsoft::WRL::ComPtr<ID3D12Resource> pNextTex,
        D3D12_CPU_DESCRIPTOR_HANDLE hPrevUavCPU, D3D12_GPU_DESCRIPTOR_HANDLE hPrevUavGPU,
        D3D12_CPU_DESCRIPTOR_HANDLE hCurrUavCPU, D3D12_GPU_DESCRIPTOR_HANDLE hCurrUavGPU,
        D3D12_CPU_DESCRIPTOR_HANDLE hNextUavCPU, D3D12_GPU_DESCRIPTOR_HANDLE hNextUavGPU,
        D3D12_CPU_DESCRIPTOR_HANDLE hCurrSrvCPU, D3D12_GPU_DESCRIPTOR_HANDLE hCurrSrvGPU
    );
};

#endif // WAVES_H