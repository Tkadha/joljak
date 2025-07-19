#include "WavesCS.h"
#include "GameFramework.h" // CGameFramework의 리소스 관리 기능 사용을 위해 포함

WavesCS::WavesCS(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList, CGameFramework* pFramework,
    int m, int n, float dx, float dt, float speed, float damping) : m_pGameFramework(pFramework)
{
    mNumRows = m;
    mNumCols = n;
    mSpatialStep = dx;

    // 파도 시뮬레이션에 사용될 상수들을 계산합니다. (CPU Waves와 동일)
    float d = damping * dt + 2.0f;
    float e = (speed * speed) * (dt * dt) / (dx * dx);
    mK1 = (damping * dt - 2.0f) / d;
    mK2 = (4.0f - 8.0f * e) / d;
    mK3 = (2.0f * e) / d;

    // 컴퓨트 셰이더 실행에 필요한 모든 리소스와 파이프라인 상태를 생성합니다.
    BuildResources(device, cmdList);
    BuildDescriptors(device, pFramework);
    BuildRootSignature(device);
    BuildPSO(device);
}

WavesCS::~WavesCS()
{
    // ComPtr이 자동으로 리소스를 해제합니다.
}

UINT WavesCS::RowCount() const { return mNumRows; }
UINT WavesCS::ColumnCount() const { return mNumCols; }
float WavesCS::Width() const { return mNumCols * mSpatialStep; }
float WavesCS::Depth() const { return mNumRows * mSpatialStep; }

// 렌더링 셰이더가 사용할 최종 결과물 텍스처의 GPU 핸들을 반환합니다.
CD3DX12_GPU_DESCRIPTOR_HANDLE WavesCS::GetDisplacementMap() const
{
    return mCurrSolSrvGpuHandle;
}

void WavesCS::BuildResources(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
    // 시뮬레이션에 사용할 텍스처의 속성을 정의합니다.
    D3D12_RESOURCE_DESC texDesc;
    ZeroMemory(&texDesc, sizeof(D3D12_RESOURCE_DESC));
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = mNumCols;
    texDesc.Height = mNumRows;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // 위치(xyz) + w, float4
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    // ★★★ 컴퓨트 셰이더가 쓰기 작업을 하려면 이 플래그가 반드시 필요합니다. ★★★
    texDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    auto heapDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

    // 3개의 텍스처 리소스(이전, 현재, 다음 상태)를 생성합니다.
    device->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&mPrevSol));
    device->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&mCurrSol));
    device->CreateCommittedResource(&heapDefault, D3D12_HEAP_FLAG_NONE, &texDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, nullptr, IID_PPV_ARGS(&mNextSol));
}

void WavesCS::BuildDescriptors(ID3D12Device* device, CGameFramework* pFramework)
{
    // SRV와 UAV를 생성하기 위한 디스크립터 정의
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MostDetailedMip = 0;
    srvDesc.Texture2D.MipLevels = 1;

    D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
    uavDesc.Texture2D.MipSlice = 0;

    // GameFramework를 통해 SRV/UAV 6개를 위한 공간을 할당
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    if (!pFramework->AllocateSrvDescriptors(6, cpuHandle, gpuHandle))
    {
        // 할당 실패
        OutputDebugStringA("FATAL ERROR: Failed to allocate descriptors for WavesCS!\n");
        return;
    }

    UINT dsvDescriptorSize = pFramework->GetCbvSrvDescriptorSize();

    // 시작 핸들부터 SRV/UAV 핸들 설정
    mPrevSolSrvCpuHandle = cpuHandle; mPrevSolSrvGpuHandle = gpuHandle; cpuHandle.ptr += dsvDescriptorSize; gpuHandle.ptr += dsvDescriptorSize;
    mCurrSolSrvCpuHandle = cpuHandle; mCurrSolSrvGpuHandle = gpuHandle; cpuHandle.ptr += dsvDescriptorSize; gpuHandle.ptr += dsvDescriptorSize;
    mNextSolSrvCpuHandle = cpuHandle; mNextSolSrvGpuHandle = gpuHandle; cpuHandle.ptr += dsvDescriptorSize; gpuHandle.ptr += dsvDescriptorSize;

    mPrevSolUavCpuHandle = cpuHandle; mPrevSolUavGpuHandle = gpuHandle; cpuHandle.ptr += dsvDescriptorSize; gpuHandle.ptr += dsvDescriptorSize;
    mCurrSolUavCpuHandle = cpuHandle; mCurrSolUavGpuHandle = gpuHandle; cpuHandle.ptr += dsvDescriptorSize; gpuHandle.ptr += dsvDescriptorSize;
    mNextSolUavCpuHandle = cpuHandle; mNextSolUavGpuHandle = gpuHandle;

    // 각 리소스에 대한 SRV와 UAV를 디스크립터 힙에 생성
    device->CreateShaderResourceView(mPrevSol.Get(), &srvDesc, mPrevSolSrvCpuHandle);
    device->CreateShaderResourceView(mCurrSol.Get(), &srvDesc, mCurrSolSrvCpuHandle);
    device->CreateShaderResourceView(mNextSol.Get(), &srvDesc, mNextSolSrvCpuHandle);

    device->CreateUnorderedAccessView(mPrevSol.Get(), nullptr, &uavDesc, mPrevSolUavCpuHandle);
    device->CreateUnorderedAccessView(mCurrSol.Get(), nullptr, &uavDesc, mCurrSolUavCpuHandle);
    device->CreateUnorderedAccessView(mNextSol.Get(), nullptr, &uavDesc, mNextSolUavCpuHandle);
}


void WavesCS::BuildRootSignature(ID3D12Device* device)
{
    CD3DX12_DESCRIPTOR_RANGE uavTable0;
    uavTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0); // u0

    CD3DX12_DESCRIPTOR_RANGE srvTable0;
    srvTable0.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0); // t0, t1

    CD3DX12_ROOT_PARAMETER rootParameters[3];
    rootParameters[0].InitAsConstants(9, 0); // b0
    rootParameters[1].InitAsDescriptorTable(1, &uavTable0);
    rootParameters[2].InitAsDescriptorTable(1, &srvTable0);

    CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(_countof(rootParameters), rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_NONE);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
    device->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&mRootSignature));
}

void WavesCS::BuildPSO(ID3D12Device* device)
{
    Microsoft::WRL::ComPtr<ID3DBlob> csBlob = nullptr;
    D3D12_SHADER_BYTECODE csByteCode = CShader::CompileShaderFromFile(L"WavesShader.hlsl", "WaveSimCS", "cs_5_1", &csBlob);

    if (csBlob == nullptr)
    {
        OutputDebugStringA("FATAL ERROR: Failed to compile compute shader 'WaveSimCS' from 'WavesShader.hlsl'.\n");
        return;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC computePsoDesc = {};
    computePsoDesc.pRootSignature = mRootSignature.Get();
    computePsoDesc.CS = csByteCode;
    computePsoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

    ThrowIfFailed(device->CreateComputePipelineState(&computePsoDesc, IID_PPV_ARGS(&mPSO)));
}


void WavesCS::Update(ID3D12GraphicsCommandList* cmdList, float time, CGameFramework* pFramework)
{
    static float t = 0.0f;
    t += time;

    // 시뮬레이션 시간 간격에 맞춰서만 컴퓨트 셰이더를 실행합니다.
    if (t >= (1.0f / 60.0f)) // 예: 60fps로 시뮬레이션
    {
        cmdList->SetPipelineState(mPSO.Get());
        cmdList->SetComputeRootSignature(mRootSignature.Get());

        cmdList->SetComputeRoot32BitConstant(0, *(UINT*)&mK1, 0);
        cmdList->SetComputeRoot32BitConstant(0, *(UINT*)&mK2, 1);
        cmdList->SetComputeRoot32BitConstant(0, *(UINT*)&mK3, 2);
        cmdList->SetComputeRoot32BitConstant(0, *(UINT*)&t, 3);
        cmdList->SetComputeRoot32BitConstant(0, mNumCols, 7);
        cmdList->SetComputeRoot32BitConstant(0, mNumRows, 8);


        auto resourceBarrier1 = CD3DX12_RESOURCE_BARRIER::Transition(mCurrSol.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
        auto resourceBarrier2 = CD3DX12_RESOURCE_BARRIER::Transition(mPrevSol.Get(), D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_GENERIC_READ);
        cmdList->ResourceBarrier(1, &resourceBarrier1);
        cmdList->ResourceBarrier(1, &resourceBarrier2);

        cmdList->SetComputeRootDescriptorTable(1, mNextSolUavGpuHandle); 
        cmdList->SetComputeRootDescriptorTable(2, mPrevSolSrvGpuHandle);


        UINT numGroupsX = (UINT)ceilf((float)mNumCols / 16.0f);
        UINT numGroupsY = (UINT)ceilf((float)mNumRows / 16.0f);
        cmdList->Dispatch(numGroupsX, numGroupsY, 1);

        auto uavBarrier = CD3DX12_RESOURCE_BARRIER::UAV(mNextSol.Get());
        cmdList->ResourceBarrier(1, &uavBarrier);

        auto transitionBarrier = CD3DX12_RESOURCE_BARRIER::Transition(mNextSol.Get(),
            D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
            D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
        cmdList->ResourceBarrier(1, &transitionBarrier);

        std::swap(mPrevSol, mCurrSol);
        std::swap(mCurrSol, mNextSol);
        std::swap(mPrevSolSrvGpuHandle, mCurrSolSrvGpuHandle);
        std::swap(mCurrSolSrvGpuHandle, mNextSolSrvGpuHandle);
        std::swap(mPrevSolUavGpuHandle, mCurrSolUavGpuHandle);
        std::swap(mCurrSolUavGpuHandle, mNextSolUavGpuHandle);

        t = 0.0f;
    }
}


void WavesCS::Disturb(ID3D12GraphicsCommandList* cmdList, UINT i, UINT j, float magnitude)
{
    // Disturb 기능은 컴퓨트 셰이더로 구현하기에는 다소 복잡하므로,
    // 이 예제에서는 생략하거나 별도의 업데이트 셰이더를 만들어야 합니다.
    // 지금은 주기적으로 파동을 일으키는 기능은 잠시 보류합니다.
}