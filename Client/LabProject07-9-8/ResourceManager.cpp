#include "ResourceManager.h"
#include "Texture.h"       // ���� CTexture ��� ��η� ����
#include "GameFramework.h" // ���� CGameFramework ��� ��η� ����
#include <iostream>        // ���� �α� ��

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
    // 1. ĳ�� Ȯ��
    auto findIter = m_TextureCache.find(filename);
    if (findIter != m_TextureCache.end())
    {
        // ĳ�ÿ� ������ shared_ptr �����Ͽ� ��ȯ 
        return findIter->second;
    }

    // 2. ĳ�ÿ� ���� -> ���� �ε�
    OutputDebugStringW((L"Loading Texture: " + filename + L"\n").c_str());

    // CTexture ��ü ����
    auto newTexture = std::make_shared<CTexture>();
    if (!newTexture) {
        OutputDebugStringW((L"Error: Failed to create shared_ptr<CTexture> for " + filename + L"\n").c_str());
        return nullptr; // �Ǵ� �� shared_ptr ��ȯ: std::shared_ptr<CTexture>();
    }

    // 3. �ε� �õ� (CTexture ���ο��� ���ҽ� �ε� �� Ÿ�� ����)
    if (!newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str()))
    {
        OutputDebugStringW((L"Error: CTexture::LoadTextureFromDDSFile failed for: " + filename + L"\n").c_str());
        return nullptr; // �ε� ���� �� nullptr (�� shared_ptr) ��ȯ
    }

    // 4. ĳ�ÿ� �߰� �� shared_ptr ��ȯ
    m_TextureCache[filename] = newTexture; // ĳ�ÿ� shared_ptr ����
    return newTexture; // ���� ������ shared_ptr ��ȯ
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
    // m_ResourceCache[resourceName] = pResource; // �ʿ�� ĳ��
    return pResource;
}

Microsoft::WRL::ComPtr<ID3D12Resource> CreateDefaultBuffer(
    ID3D12Device* device,
    ID3D12GraphicsCommandList* cmdList,
    const void* initData,
    UINT64 byteSize,
    Microsoft::WRL::ComPtr<ID3D12Resource>& uploadBuffer, // ������ ���ε� ���۸� ��ȯ (ȣ���ڰ� �����ϰų� ����)
    D3D12_RESOURCE_STATES finalState = D3D12_RESOURCE_STATE_COMMON) // ���� ���ҽ� ����
{
    Microsoft::WRL::ComPtr<ID3D12Resource> defaultBuffer;

    // 1. �⺻ ��(Default Heap)�� ���� ���� ����
    auto heapPropsDefault = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
    auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
    HRESULT hr = device->CreateCommittedResource(
        &heapPropsDefault,
        D3D12_HEAP_FLAG_NONE,
        &bufferDesc,
        D3D12_RESOURCE_STATE_COPY_DEST, // ó������ ���� ������� ���� ����
        nullptr,
        IID_PPV_ARGS(defaultBuffer.GetAddressOf()));
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to create default buffer.\n");
        return nullptr;
    }

    if (initData)
    {
        // 2. ���ε� ��(Upload Heap)�� �ӽ� ���� ����
        auto heapPropsUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        hr = device->CreateCommittedResource(
            &heapPropsUpload,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc, // ���ε� ���۵� ���� ũ��
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr,
            IID_PPV_ARGS(uploadBuffer.GetAddressOf()));
        if (FAILED(hr)) {
            OutputDebugStringA("Failed to create upload buffer.\n");
            return nullptr; // defaultBuffer�� �̹� �����Ǿ����Ƿ�, �����δ� �� ������ ���� ó�� �ʿ�
        }

        // 3. ������ ���� (CPU -> Upload Heap)
        D3D12_SUBRESOURCE_DATA subResourceData = {};
        subResourceData.pData = initData;
        subResourceData.RowPitch = byteSize;
        subResourceData.SlicePitch = subResourceData.RowPitch;

        // UpdateSubresources ���� �Լ� ��� (d3dx12.h �� ����)
        // defaultBuffer�� COPY_DEST ���·�, uploadBuffer�� GENERIC_READ ���·� �� ä ����
        // ���� �� defaultBuffer�� finalState�� �����ϴ� �踮� �����
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
            D3D12_RESOURCE_STATE_COPY_DEST, finalState);

        UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
        cmdList->ResourceBarrier(1, &barrier);
    }
    else // �ʱ� �����Ͱ� ���ٸ�, ���� ���·� �ٷ� ����
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
    bool bIsDynamic, // ���� ���� �÷���
    D3D12_RESOURCE_STATES initialResourceState // ���� ������ ���� ����
)
{
    UINT64 byteSize = static_cast<UINT64>(uiElementSize) * uiCount;
    Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBufferHolder; // CreateDefaultBuffer�� ����� ���ε� ����

    // bIsDynamic �÷��� ó���� ���⼭ �� ��üȭ�� �� �ֽ��ϴ�.
    // 1. �ſ� ���� CPU���� ������Ʈ: D3D12_HEAP_TYPE_UPLOAD�� ���� ����
    // 2. ���� ������Ʈ: D3D12_HEAP_TYPE_DEFAULT�� �����ϰ� UpdateDynamicBuffer ���
    // ���⼭�� �켱 Default ���� �����ϴ� ������ �����ϰ�, UpdateDynamicBuffer���� ó���մϴ�.
    if (bIsDynamic) {
        // ���� ���۸� UPLOAD ���� ���� ���� ��� (Map/Unmap���� ������Ʈ)
        auto heapPropsUpload = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
        auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);
        Microsoft::WRL::ComPtr<ID3D12Resource> dynamicVertexBuffer;
        HRESULT hr = pd3dDevice->CreateCommittedResource(
            &heapPropsUpload,
            D3D12_HEAP_FLAG_NONE,
            &bufferDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, // ���ε� ���� �׻� GENERIC_READ
            nullptr,
            IID_PPV_ARGS(&dynamicVertexBuffer)
        );
        if (FAILED(hr)) {
            OutputDebugStringW((L"Failed to create dynamic vertex buffer (upload heap): " + name + L"\n").c_str());
            return nullptr;
        }
        dynamicVertexBuffer->SetName((name + L"_VB_Dynamic_Upload").c_str());

        if (pData) { // �ʱ� �����Ͱ� �ִٸ� ����
            void* pMappedData = nullptr;
            CD3DX12_RANGE readRange(0, 0); // ���� ����
            hr = dynamicVertexBuffer->Map(0, &readRange, &pMappedData);
            if (SUCCEEDED(hr)) {
                memcpy(pMappedData, pData, byteSize);
                dynamicVertexBuffer->Unmap(0, nullptr);
            }
        }
        // m_ResourceCache[name] = dynamicVertexBuffer; // �ʿ�� ĳ��
        return dynamicVertexBuffer;
    }
    else {
        // ���� �Ǵ� ���� ������Ʈ�Ǵ� ���۴� Default ���� ����
        Microsoft::WRL::ComPtr<ID3D12Resource> defaultVertexBuffer = CreateDefaultBuffer(
            pd3dDevice,
            pd3dCommandList,
            pData,
            byteSize,
            pUploadBufferHolder, // �ӽ� ���ε� ���� (�� �Լ� ȣ�� �� GPU ���� �Ϸ� �������� ��ȿ�ؾ� ��)
            initialResourceState // ���� D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER
        );

        if (defaultVertexBuffer) {
            defaultVertexBuffer->SetName((name + L"_VB_Default").c_str());
        }
        // m_ResourceCache[name] = defaultVertexBuffer; // �ʿ�� ĳ��
        // pUploadBufferHolder�� GPU �۾� �Ϸ� �� �����Ǿ�� �� (������ ���ҽ� �ý��� �Ǵ� ����ȭ �ʿ�)
        return defaultVertexBuffer;
    }
}

Microsoft::WRL::ComPtr<ID3D12Resource> ResourceManager::CreateIndexBuffer(
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    const std::wstring& name,
    const void* pData, // �ε��� ������
    UINT uiCount,      // �ε��� ���� (uiElementSize�� DXGI_FORMAT���� ������)
    D3D12_RESOURCE_STATES initialResourceState // �ε��� ������ ���� ����
)
{
    // �ε��� ũ��� ���� 2����Ʈ(DXGI_FORMAT_R16_UINT) �Ǵ� 4����Ʈ(DXGI_FORMAT_R32_UINT)
    // ���⼭�� uint32_t (4����Ʈ)�� ����. ���� ������ Ÿ�Կ� ����� ��.
    UINT64 byteSize = static_cast<UINT64>(sizeof(uint32_t)) * uiCount;
    if (pData && uiCount > 0) {
        // �����δ� �ε��� ����(R16 or R32)�� �Ķ���ͷ� �ްų�,
        // pData�� Ÿ�����κ��� sizeof�� �����ؾ� ��.
        // ��: if (indexBufferFormat == DXGI_FORMAT_R16_UINT) byteSize = sizeof(uint16_t) * uiCount;
    }


    Microsoft::WRL::ComPtr<ID3D12Resource> pUploadBufferHolder;

    Microsoft::WRL::ComPtr<ID3D12Resource> defaultIndexBuffer = CreateDefaultBuffer(
        pd3dDevice,
        pd3dCommandList,
        pData,
        byteSize,
        pUploadBufferHolder,
        initialResourceState // ���� D3D12_RESOURCE_STATE_INDEX_BUFFER
    );

    if (defaultIndexBuffer) {
        defaultIndexBuffer->SetName((name + L"_IB_Default").c_str());
    }
    // m_ResourceCache[name] = defaultIndexBuffer; // �ʿ�� ĳ��
    // pUploadBufferHolder�� GPU �۾� �Ϸ� �� �����Ǿ�� ��
    return defaultIndexBuffer;
}

void ResourceManager::UpdateDynamicBuffer(
    ID3D12Resource* pBufferToUpdate, // ������Ʈ�� ��� ���� (Default ���� �ִٰ� ����)
    const void* pData,
    UINT uiSize,
    ID3D12GraphicsCommandList* pd3dCommandList, // �� Ŀ�ǵ� ����Ʈ���� ���� ��� ����
    ID3D12Device* pd3dDevice // �ӽ� ���ε� ���� ������
)
{
    if (!pBufferToUpdate || !pData || uiSize == 0 || !pd3dCommandList || !pd3dDevice) {
        assert("Invalid arguments for UpdateDynamicBuffer");
        return;
    }

    // 1. �ӽ� ���ε� ���� ����
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


    // 2. ������ ���� (CPU -> �ӽ� ���ε� ����)
    void* pMappedData = nullptr;
    CD3DX12_RANGE readRange(0, 0); // We do not intend to read from this resource on the CPU.
    hr = tempUploadBuffer->Map(0, &readRange, &pMappedData);
    if (FAILED(hr)) {
        OutputDebugStringA("Failed to map temporary upload buffer.\n");
        return;
    }
    memcpy(pMappedData, pData, uiSize);
    tempUploadBuffer->Unmap(0, nullptr);


    // 3. ��� ���� ���� ���� (COPY_DEST��)
    //    (��� ������ ���� ���¸� �˾ƾ� ��Ȯ�� Transition�� ������, ������ ��� �� GENERIC_READ �Ǵ� Ư�� ��� ������ ����)
    //    ���� �Ϲ����� VertexBuffer ���¿��� COPY_DEST�� �����Ѵٰ� ����
    auto preCopyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pBufferToUpdate,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // �Ǵ� D3D12_RESOURCE_STATE_INDEX_BUFFER �� ���� ����
        D3D12_RESOURCE_STATE_COPY_DEST
    );
    pd3dCommandList->ResourceBarrier(1, &preCopyBarrier);

    // 4. ���� ��� ��� (�ӽ� ���ε� ���� -> ��� ����)
    pd3dCommandList->CopyBufferRegion(pBufferToUpdate, 0, tempUploadBuffer.Get(), 0, uiSize);

    // 5. ��� ���� ���� ���� (���� ��� ���·�)
    auto postCopyBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        pBufferToUpdate,
        D3D12_RESOURCE_STATE_COPY_DEST,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER // �Ǵ� D3D12_RESOURCE_STATE_INDEX_BUFFER �� ���� ����
    );
    pd3dCommandList->ResourceBarrier(1, &postCopyBarrier);

    // �߿�: tempUploadBuffer�� �� pd3dCommandList�� GPU���� ���� �Ϸ�� ������ �����Ǹ� �� �˴ϴ�.
    // CGameFramework�� ������ ���ҽ� �ý��� ���� ���� �����ϰų�,
    // Ŀ�ǵ� ���� �� ���������� ��ٸ��ٸ� ���⼭ ComPtr�� �������� ����� ������ �� �ֽ��ϴ�.
    // ��: m_pGameFramework->AddResourceToDeferDelete(tempUploadBuffer);
}