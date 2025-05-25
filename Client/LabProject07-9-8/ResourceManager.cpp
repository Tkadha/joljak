#include "ResourceManager.h"
#include "Texture.h"       // 실제 CTexture 헤더 경로로 수정
#include "GameFramework.h" // 실제 CGameFramework 헤더 경로로 수정
#include <iostream>        // 오류 로깅 등

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager() 
{
    ReleaseAll();
}

void ResourceManager::ReleaseAll() {
    m_TextureCache.clear();
}

bool ResourceManager::Initialize(CGameFramework* pFramework)
{
    if (!pFramework) return false;
    m_pFramework = pFramework;
    return true;
}

std::shared_ptr<CTexture> ResourceManager::GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList)
{
    // 1. 캐시 확인
    auto findIter = m_TextureCache.find(filename);
    if (findIter != m_TextureCache.end())
    {
        // 캐시에 있으면 shared_ptr 복사하여 반환 
        return findIter->second;
    }

    // 2. 캐시에 없음 -> 새로 로드
    OutputDebugStringW((L"Loading Texture: " + filename + L"\n").c_str());

    // CTexture 객체 생성
    auto newTexture = std::make_shared<CTexture>();
    if (!newTexture) {
        OutputDebugStringW((L"Error: Failed to create shared_ptr<CTexture> for " + filename + L"\n").c_str());
        return nullptr; // 또는 빈 shared_ptr 반환: std::shared_ptr<CTexture>();
    }

    // 3. 로드 시도 (CTexture 내부에서 리소스 로딩 및 타입 결정)
    if (!newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str()))
    {
        OutputDebugStringW((L"Error: CTexture::LoadTextureFromDDSFile failed for: " + filename + L"\n").c_str());
        return nullptr; // 로드 실패 시 nullptr (빈 shared_ptr) 반환
    }

    // 4. 캐시에 추가 및 shared_ptr 반환
    m_TextureCache[filename] = newTexture; // 캐시에 shared_ptr 저장
    return newTexture; // 새로 생성된 shared_ptr 반환
}


Microsoft::WRL::ComPtr<ID3D12Resource> ResourceManager::CreateUAVTexture2D(
    ID3D12Device* pd3dDevice, const std::wstring& resourceName,
    UINT width, UINT height, DXGI_FORMAT format,
    D3D12_RESOURCE_STATES initialResourceState, const D3D12_CLEAR_VALUE* pOptimizedClearValue,
    D3D12_RESOURCE_FLAGS flags)
{
    Microsoft::WRL::ComPtr<ID3D12Resource> pResource;
    D3D12_RESOURCE_DESC texDesc = {};
    texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    texDesc.Alignment = 0;
    texDesc.Width = width;
    texDesc.Height = height;
    texDesc.DepthOrArraySize = 1;
    texDesc.MipLevels = 1;
    texDesc.Format = format;
    texDesc.SampleDesc.Count = 1;
    texDesc.SampleDesc.Quality = 0;
    texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    texDesc.Flags = flags;

    auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    HRESULT hr = pd3dDevice->CreateCommittedResource(
        &heapProps,
        D3D12_HEAP_FLAG_NONE,
        &texDesc,
        initialResourceState,
        pOptimizedClearValue,
        IID_PPV_ARGS(&pResource));

    if (FAILED(hr)) {
        OutputDebugStringW((L"Failed to create UAV Texture2D: " + resourceName + L"\n").c_str());
        return nullptr;
    }
    pResource->SetName((resourceName + L"_UAV_Texture").c_str());
    // m_ResourceCache[resourceName] = pResource; // 필요시 캐싱
    return pResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, // 생성된 업로드 버퍼를 반환 (호출자가 해제하거나 재사용)
    D3D12_RESOURCE_STATES finalState = D3D12_RESOURCE_STATE_COMMON) // 최종 리소스 상태
{
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

    // 1. 기본 힙(Default Heap)에 실제 버퍼 생성
    auto heapPropsDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    HRESULT hr = device->CreateCommittedResource(
        &heapPropsDefault,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, // 처음에는 복사 대상으로 상태 설정
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create default buffer.\n");
        return nullptr;
    }

    if (initData)
    {
        // 2. 업로드 힙(Upload Heap)에 임시 버퍼 생성
        auto heapPropsUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        hr = device->CreateCommittedResource(
            &heapPropsUpload,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc, // 업로드 버퍼도 같은 크기
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf()));
        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create upload buffer.\n");
            return nullptr; // defaultBuffer는 이미 생성되었으므로, 실제로는 더 정교한 에러 처리 필요
        }

        // 3. 데이터 복사 (CPU -> Upload Heap)
        D3D12_SUBRESOURCE_DATA subResourceData = {};
        subResourceData.pData = initData;
        subResourceData.RowPitch = byteSize;
        subResourceData.SlicePitch = subResourceData.RowPitch;

        // UpdateSubresources 헬퍼 함수 사용 (d3dx12.h 에 있음)
        // defaultBuffer를 COPY_DEST 상태로, uploadBuffer를 GENERIC_READ 상태로 둔 채 복사
        // 복사 후 defaultBuffer를 finalState로 변경하는 배리어를 기록함
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, finalState);

        UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
        cmdList->ResourceBarrier(1, &barrier);
    }
    else // 초기 데이터가 없다면, 최종 상태로 바로 전이
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, finalState);
        cmdList->ResourceBarrier(1, &barrier);
    }


    return defaultBuffer;
}

Microsoft::WRL::ComPtr<ID3D12Resource> ResourceManager::CreateVertexBuffer(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    const std::wstring& name,
    const void* pData,
    UINT uiElementSize,
    UINT uiCount,
    bool bIsDynamic, // 동적 여부 플래그
    D3D12_RESOURCE_STATES initialResourceState // 정점 버퍼의 최종 상태
)
{
    UINT64 byteSize = static_cast<UINT64>(uiElementSize) * uiCount;
    Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBufferHolder; // CreateDefaultBuffer가 사용할 업로드 버퍼

    // bIsDynamic 플래그 처리는 여기서 더 구체화될 수 있습니다.
    // 1. 매우 자주 CPU에서 업데이트: D3D12_HEAP_TYPE_UPLOAD에 직접 생성
    // 2. 가끔 업데이트: D3D12_HEAP_TYPE_DEFAULT에 생성하고 UpdateDynamicBuffer 사용
    // 여기서는 우선 Default 힙에 생성하는 것으로 통일하고, UpdateDynamicBuffer에서 처리합니다.
    if (bIsDynamic) {
        // 동적 버퍼를 UPLOAD 힙에 직접 만들 경우 (Map/Unmap으로 업데이트)
        auto heapPropsUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        Microsoft::WRL::ComPtr<ID3D12Resource> dynamicVertexBuffer;
        HRESULT hr = pd3dDevice->CreateCommittedResource(
            &heapPropsUpload,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, // 업로드 힙은 항상 GENERIC_READ
            nullptr,
            IID_PPV_ARGS(&dynamicVertexBuffer)
        );
        if (FAILED(hr)) {
            OutputDebugStringW((L"Failed to create dynamic vertex buffer (upload heap): " + name + L"\n").c_str());
            return nullptr;
        }
        dynamicVertexBuffer->SetName((name + L"_VB_Dynamic_Upload").c_str());

        if (pData) { // 초기 데이터가 있다면 복사
            void* pMappedData = nullptr;
            CD3DX12_RANGE readRange(0, 0); // 읽지 않음
            hr = dynamicVertexBuffer->Map(0, &readRange, &pMappedData);
            if (SUCCEEDED(hr)) {
                memcpy(pMappedData, pData, byteSize);
                dynamicVertexBuffer->Unmap(0, nullptr);
            }
        }
        // m_ResourceCache[name] = dynamicVertexBuffer; // 필요시 캐싱
        return dynamicVertexBuffer;
    }
    else {
        // 정적 또는 가끔 업데이트되는 버퍼는 Default 힙에 생성
        Microsoft::WRL::ComPtr<ID3D12Resource> defaultVertexBuffer = CreateDefaultBuffer(
            pd3dDevice,
            pd3dCommandList,
            pData,
            byteSize,
            pUploadBufferHolder, // 임시 업로드 버퍼 (이 함수 호출 후 GPU 실행 완료 시점까지 유효해야 함)
            initialResourceState // 보통 D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        );

        if (defaultVertexBuffer) {
            defaultVertexBuffer->SetName((name + L"_VB_Default").c_str());
        }
        // m_ResourceCache[name] = defaultVertexBuffer; // 필요시 캐싱
        // pUploadBufferHolder는 GPU 작업 완료 후 해제되어야 함 (프레임 리소스 시스템 또는 동기화 필요)
        return defaultVertexBuffer;
    }
}

Microsoft::WRL::ComPtr<ID3D12Resource> ResourceManager::CreateIndexBuffer(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    const std::wstring& name,
    const void* pData, // 인덱스 데이터
    UINT uiCount,      // 인덱스 개수 (uiElementSize는 DXGI_FORMAT으로 결정됨)
    D3D12_RESOURCE_STATES initialResourceState // 인덱스 버퍼의 최종 상태
)
{
    // 인덱스 크기는 보통 2바이트(DXGI_FORMAT_R16_UINT) 또는 4바이트(DXGI_FORMAT_R32_UINT)
    // 여기서는 uint32_t (4바이트)를 가정. 실제 데이터 타입에 맞춰야 함.
    UINT64 byteSize = static_cast<UINT64>(sizeof(uint32_t)) * uiCount;
    if (pData && uiCount > 0) {
        // 실제로는 인덱스 포맷(R16 or R32)을 파라미터로 받거나,
        // pData의 타입으로부터 sizeof를 결정해야 함.
        // 예: if (indexBufferFormat == DXGI_FORMAT_R16_UINT) byteSize = sizeof(uint16_t) * uiCount;
    }


    Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBufferHolder;

    Microsoft::WRL::ComPtr<ID3D12Resource> defaultIndexBuffer = CreateDefaultBuffer(
        pd3dDevice,
        pd3dCommandList,
        pData,
        byteSize,
        pUploadBufferHolder,
        initialResourceState // 보통 D3D12_RESOURCE_STATE_INDEX_BUFFER
    );

    if (defaultIndexBuffer) {
        defaultIndexBuffer->SetName((name + L"_IB_Default").c_str());
    }
    // m_ResourceCache[name] = defaultIndexBuffer; // 필요시 캐싱
    // pUploadBufferHolder는 GPU 작업 완료 후 해제되어야 함
    return defaultIndexBuffer;
}

void ResourceManager::UpdateDynamicBuffer(
    ID3D12Resource* pBufferToUpdate, // 업데이트할 대상 버퍼 (Default 힙에 있다고 가정)
    const void* pData,
    UINT uiSize,
    ID3D12GraphicsCommandList* pd3dCommandList, // 이 커맨드 리스트에서 복사 명령 실행
    ID3D12Device* pd3dDevice // 임시 업로드 버퍼 생성용
)
{
    if (!pBufferToUpdate || !pData || uiSize == 0 || !pd3dCommandList || !pd3dDevice) {
        assert("Invalid arguments for UpdateDynamicBuffer");
        return;
    }

    // 1. 임시 업로드 버퍼 생성
    Microsoft::WRL::ComPtr<ID3D12Resource> tempUploadBuffer;
    auto heapPropsUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(uiSize);
    HRESULT hr = pd3dDevice->CreateCommittedResource(
        &heapPropsUpload,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&tempUploadBuffer)
    );
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create temporary upload buffer for dynamic update.\n");
        return;
    }
    tempUploadBuffer->SetName(L"TempUploadBuffer_ForDynamicUpdate");


    // 2. 데이터 복사 (CPU -> 임시 업로드 버퍼)
    void* pMappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    hr = tempUploadBuffer->Map(0, &readRange, &pMappedData);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to map temporary upload buffer.\n");
        return;
    }
    memcpy(pMappedData, pData, uiSize);
    tempUploadBuffer->Unmap(0, nullptr);


    // 3. 대상 버퍼 상태 전이 (COPY_DEST로)
    //    (대상 버퍼의 현재 상태를 알아야 정확한 Transition을 하지만, 보통은 사용 후 GENERIC_READ 또는 특정 사용 상태일 것임)
    //    가장 일반적인 VertexBuffer 상태에서 COPY_DEST로 변경한다고 가정
    auto preCopyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pBufferToUpdate,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // 또는 D3D12_RESOURCE_STATE_INDEX_BUFFER 등 이전 상태
        D3D12_RESOURCE_STATE_COPY_DEST
    );
    pd3dCommandList->ResourceBarrier(1, &preCopyBarrier);

    // 4. 복사 명령 기록 (임시 업로드 버퍼 -> 대상 버퍼)
    pd3dCommandList->CopyBufferRegion(pBufferToUpdate, 0, tempUploadBuffer.Get(), 0, uiSize);

    // 5. 대상 버퍼 상태 복구 (원래 사용 상태로)
    auto postCopyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pBufferToUpdate,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER // 또는 D3D12_RESOURCE_STATE_INDEX_BUFFER 등 원래 상태
    );
    pd3dCommandList->ResourceBarrier(1, &postCopyBarrier);

    // 중요: tempUploadBuffer는 이 pd3dCommandList가 GPU에서 실행 완료될 때까지 해제되면 안 됩니다.
    // CGameFramework의 프레임 리소스 시스템 등을 통해 관리하거나,
    // 커맨드 실행 후 동기적으로 기다린다면 여기서 ComPtr이 스코프를 벗어나며 해제될 수 있습니다.
    // 예: m_pGameFramework->AddResourceToDeferDelete(tempUploadBuffer);
}