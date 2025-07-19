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

    // 복사 및 대입 방지
    WavesCS(const WavesCS& rhs) = delete;
    WavesCS& operator=(const WavesCS& rhs) = delete;

    UINT RowCount() const;
    UINT ColumnCount() const;
    float Width() const;
    float Depth() const;

    // 컴퓨트 셰이더가 계산한 최종 결과물(텍스처)의 SRV 핸들을 반환합니다.
    // 이 핸들은 렌더링 셰이더에서 물 표면을 그릴 때 사용됩니다.
    CD3DX12_GPU_DESCRIPTOR_HANDLE GetDisplacementMap() const;

    // 매 프레임 호출되어 컴퓨트 셰이더를 실행하고 파도를 업데이트합니다.
    void Update(ID3D12GraphicsCommandList* cmdList, float time, CGameFramework* pFramework);

    // 특정 위치에 물결을 생성합니다.
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

    // 파도 시뮬레이션 상수 
    float mK1 = 0.0f;
    float mK2 = 0.0f;
    float mK3 = 0.0f;

    // 컴퓨트 셰이더와 PSO
    Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature = nullptr;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> mPSO = nullptr;

    // 시뮬레이션에 사용할 3개의 텍스처 리소스 (이전, 현재, 다음)
    Microsoft::WRL::ComPtr<ID3D12Resource> mPrevSol = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> mCurrSol = nullptr;
    Microsoft::WRL::ComPtr<ID3D12Resource> mNextSol = nullptr;

    // 각 텍스처에 대한 뷰(View) 핸들
    // SRV: 셰이더가 읽기용으로 사용
    // UAV: 셰이더가 쓰기용으로 사용
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