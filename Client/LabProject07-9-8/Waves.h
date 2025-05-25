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

    // �ùķ��̼� ����� ���� (CPU ������)
    const DirectX::XMFLOAT3& Position(int i) const { return mCurrSolution[i]; }
    const DirectX::XMFLOAT3& Normal(int i) const { return mNormals[i]; }
    const DirectX::XMFLOAT3& TangentX(int i) const { return mTangentX[i]; } // �ʿ��ϴٸ�

    // GPU ���ҽ� ID �Ǵ� �ڵ� (ResourceManager�� ����)
    // ����: ���� ID Ÿ���� ResourceManager ���迡 ���� �ٸ�
    size_t GetPrevSolUAV() const { return mPrevSolUAVHandle; }
    size_t GetCurrSolUAV() const { return mCurrSolUAVHandle; }
    size_t GetNextSolUAV() const { return mNextSolUAVHandle; }

    // CurrSol�� ������ �� SRV�ε� ���� �� ����
    size_t GetCurrSolSRV() const { return mCurrSolSRVHandle; }

    void Update(float dt);
    void Disturb(int i, int j, float magnitude);

    // �ʱ�ȭ (ResourceManager�� ���� GPU ���ҽ� ���� ��û)
   // �� �Լ��� ResourceManager�� Device ������ ������ ������ ȣ��Ǿ�� ��
   // bool InitResources(ID3D12Device* device, ResourceManager* resManager);
   // �Ǵ� Scene/GameFramework���� ���� ���ҽ��� �����ϰ� �ڵ��� Waves ��ü�� �Ѱ��� ���� ����
    void SetResourceHandles(size_t prevSolUAV, size_t currSolUAV, size_t nextSolUAV, size_t currSolSRV);


    // Simulation constants we can precompute.
    float mK1 = 0.0f;
    float mK2 = 0.0f;
    float mK3 = 0.0f;
    float mTimeStep = 0.0f;     // public���� ���� �Ǵ� getter �߰�
    float mSpatialStep = 0.0f;  // public���� ���� �Ǵ� getter �߰�


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
    size_t mCurrSolSRVHandle = -1; // CurrSol�� SRV�ε� ����� ���


public:
    // GPU ���ҽ� (UAV�� ���� �ؽ�ó��)
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pPrevSolTexture;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pCurrSolTexture;
    Microsoft::WRL::ComPtr<ID3D12Resource> m_pNextSolTexture;

    // �� ���ҽ��� ���� ��ũ���� �ڵ�
    D3D12_CPU_DESCRIPTOR_HANDLE m_hPrevSolUavCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hPrevSolUavGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE m_hCurrSolUavCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hCurrSolUavGPU;
    D3D12_CPU_DESCRIPTOR_HANDLE m_hNextSolUavCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hNextSolUavGPU;

    // m_pCurrSolTexture�� ������ �� SRV�ε� ����
    D3D12_CPU_DESCRIPTOR_HANDLE m_hCurrSolSrvCPU;
    D3D12_GPU_DESCRIPTOR_HANDLE m_hCurrSolSrvGPU;

    // ���� ������ ���� ���� ������ �ڵ� �� ���ҽ� ��ȯ �Լ��� (���� �ʿ�)
    ID3D12Resource* GetCurrentPrevSolTexture() const;
    ID3D12Resource* GetCurrentCurrSolTexture() const;
    ID3D12Resource* GetCurrentNextSolTexture() const; // ��ǻƮ ���̴��� ��� ���
    ID3D12Resource* GetCurrentCurrSolTextureForRendering() const; // ������ �� SRV�� ���� �ؽ�ó

    D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentPrevSolUAV_GPU() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentCurrSolUAV_GPU() const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetCurrentNextSolUAV_GPU() const;
    // ��ǻƮ ���̴� ��Ʈ �ñ״�ó�� UAV 3���� �ϳ��� ���̺�� �޴´ٸ�, �� ���� �ڵ�
    D3D12_GPU_DESCRIPTOR_HANDLE GetUAVDescriptorTableStartGPU() const;


    void SwapSimBuffersAndDescriptors(); // ������ ���� ���� �� �ڵ� ��ü ����

private:
    // ���� ������ ���� ���� �ε��� �Ǵ� ���� ����
    int m_nCurrentBufferIndex = 0; // ����: 0, 1, 2 �� ��ȯ�ϸ� Prev, Curr, Next ���� ����
    // �Ǵ� ComPtr<ID3D12Resource> �迭�� �ڵ� �迭�� �ΰ� �ε����� ����4

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