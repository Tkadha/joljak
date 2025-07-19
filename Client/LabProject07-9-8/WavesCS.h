#pragma once
#include "stdafx.h"
#include <string>
#include <vector>

class CGameFramework;

class WavesCS
{
public:
    WavesCS(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pFramework,
        int m, int n, float dx, float dt, float speed, float damping);
    ~WavesCS();

    // ���� �� ���� ����
    WavesCS(const WavesCS& rhs) = delete;
    WavesCS& operator=(const WavesCS& rhs) = delete;

    UINT RowCount() const;
    UINT ColumnCount() const;
    float Width() const;
    float Depth() const;

    // ��ǻƮ ���̴��� ����� ���� �����(�ؽ�ó)�� SRV �ڵ��� ��ȯ�մϴ�.
    // �� �ڵ��� ������ ���̴����� �� ǥ���� �׸� �� ���˴ϴ�.
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetDisplacementMap() const;

    // �� ������ ȣ��Ǿ� ��ǻƮ ���̴��� �����ϰ� �ĵ��� ������Ʈ�մϴ�.
    void Update(ID3D12GraphicsCommandList* cmdList, float time, CGameFramework* pFramework);

    // Ư�� ��ġ�� ������ �����մϴ�.
    void Disturb(ID3D12GraphicsCommandList* cmdList, UINT i, UINT j, float magnitude);

private:
    void BuildResources(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList);
    void BuildDescriptors(ID3D12Device* device, CGameFramework* pFramework);
    void BuildRootSignature(ID3D12Device* device);
    void BuildPSO(ID3D12Device* device);

private:
    CGameFramework* m_pGameFramework = nullptr;

    UINT mNumRows = 0;
    UINT mNumCols = 0;

    float mSpatialStep = 0.0f;

    // �ĵ� �ùķ��̼� ��� 
    float mK1 = 0.0f;
    float mK2 = 0.0f;
    float mK3 = 0.0f;

    // ��ǻƮ ���̴��� PSO
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;

    // �ùķ��̼ǿ� ����� 3���� �ؽ�ó ���ҽ� (����, ����, ����)
    Microsoft::WRL::ComPtr<ID3D12Resource> mPrevSol = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> mCurrSol = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> mNextSol = nullptr;

    // �� �ؽ�ó�� ���� ��(View) �ڵ�
    // SRV: ���̴��� �б������ ���
    // UAV: ���̴��� ��������� ���
    CD3DX12_CPU_DESCRIPTOR_HANDLE mPrevSolSrvCpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mPrevSolSrvGpuHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mPrevSolUavCpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mPrevSolUavGpuHandle;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mCurrSolSrvCpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mCurrSolSrvGpuHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mCurrSolUavCpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mCurrSolUavGpuHandle;

    CD3DX12_CPU_DESCRIPTOR_HANDLE mNextSolSrvCpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mNextSolSrvGpuHandle;
    CD3DX12_CPU_DESCRIPTOR_HANDLE mNextSolUavCpuHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE mNextSolUavGpuHandle;
};