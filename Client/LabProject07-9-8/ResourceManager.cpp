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