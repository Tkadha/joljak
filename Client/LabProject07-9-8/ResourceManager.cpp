#include "ResourceManager.h"
#include "Texture.h"       // ���� CTexture ��� ��η� ����
#include "GameFramework.h" // ���� CGameFramework ��� ��η� ����
#include <iostream>        // ���� �α� ��

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
    // unique_ptr�� �ڵ����� �޸� ����
}

bool ResourceManager::Initialize(CGameFramework* pFramework)
{
    if (!pFramework) return false;
    m_pFramework = pFramework;
    return true;
}

CTexture* ResourceManager::GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList)
{
    // 1. ĳ�� Ȯ��
    auto findIter = m_TextureCache.find(filename);
    if (findIter != m_TextureCache.end())
    {
        // ĳ�ÿ� �����ϸ� �ٷ� ��ȯ
        return findIter->second.get();
    }

    // 2. ĳ�ÿ� ���� -> ���� �ε�
    OutputDebugStringW((L"Loading Texture: " + filename + L"\n").c_str()); // �ε� �α�

    // �� CTexture ��ü ���� (unique_ptr ���)
    // CTexture ������ �Ķ���ʹ� ���� ���ǿ� �°� ���� �ʿ�
    auto newTexture = std::make_unique<CTexture>(1, RESOURCE_TEXTURE2D, 0, 1);
    if (!newTexture)
    {
        OutputDebugStringW((L"Error: Failed to create CTexture object for " + filename + L"\n").c_str());
        return nullptr;
    }

    // 3. DDS ���� �ε��Ͽ� ID3D12Resource ����
    // LoadTextureFromDDSFile �Լ��� ���� �� true �Ǵ� HRESULT S_OK ��ȯ ����
    newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str(), RESOURCE_TEXTURE2D, 0);
    //if (!newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str(), RESOURCE_TEXTURE2D, 0))
    //{
    //    OutputDebugStringW((L"Error: Failed to load DDS file: " + filename + L"\n").c_str());
    //    return nullptr; // �ε� ���� �� nullptr ��ȯ
    //}

    // 4. SRV ����
    int nTextures = newTexture->GetTextures(); // ���� DDS�� 1��, �ؽ�ó �迭/ť����� ���� ��
    if (nTextures > 0)
    {
        D3D12_CPU_DESCRIPTOR_HANDLE cpuStartHandle = { 0 };
        D3D12_GPU_DESCRIPTOR_HANDLE gpuStartHandle = { 0 };

        // 4a. �����ӿ�ũ���� SRV ��ũ���� ���� �Ҵ� ��û
        if (!m_pFramework->AllocateSrvDescriptors(nTextures, cpuStartHandle, gpuStartHandle))
        {
            OutputDebugStringW((L"Error: Failed to allocate SRV descriptors for " + filename + L"\n").c_str());
            // ���ҽ��� �ε������� SRV ������ ���ϴ� ��Ȳ. �ε�� ���ҽ� ���� �ʿ��� �� ����.
            return nullptr; // ���� ó��
        }

        // 4b. �Ҵ�� ���Կ� SRV ����
        D3D12_CPU_DESCRIPTOR_HANDLE currentCpuHandle = cpuStartHandle;
        D3D12_GPU_DESCRIPTOR_HANDLE currentGpuHandle = gpuStartHandle; // GPU �ڵ鵵 �����ϱ� ����

        for (int i = 0; i < nTextures; ++i)
        {
            ID3D12Resource* pShaderResource = newTexture->GetResource(i);
            D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = newTexture->GetShaderResourceViewDesc(i);

            if (pShaderResource)
            {
                m_pFramework->GetDevice()->CreateShaderResourceView(pShaderResource, &srvDesc, currentCpuHandle);
            }
            else
            {
                // ���ҽ��� ���� ��� ó�� (Null SRV ���� ��)
                m_pFramework->GetDevice()->CreateShaderResourceView(nullptr, &srvDesc, currentCpuHandle); // ��: Null SRV
            }


            // CTexture ��ü�� �ش� SRV�� GPU �ڵ� ����
            newTexture->SetGpuDescriptorHandle(i, currentGpuHandle);

            // ���� �������� �̵�
            currentCpuHandle.ptr += m_pFramework->GetCbvSrvDescriptorSize();
            currentGpuHandle.ptr += m_pFramework->GetCbvSrvDescriptorSize();
        }
        // CTexture ��ü�� ���� GPU �ڵ鸸 �����ϰ� �ʹٸ� �Ʒ�ó�� ����
        // newTexture->SetBaseGpuDescriptorHandle(gpuStartHandle); // �̷� �Լ��� �ִٰ� ����
    }

    // 5. �ε� �� SRV ���� �Ϸ�� �ؽ�ó�� ĳ�ÿ� �߰��ϰ� ������ ��ȯ
    // newTexture�� �������� ĳ��(m_TextureCache)�� �̵�(move)
    auto [iter, success] = m_TextureCache.emplace(filename, std::move(newTexture));
    if (!success) {
        OutputDebugStringW((L"Error: Failed to insert texture into cache: " + filename + L"\n").c_str());
        return nullptr; // ĳ�� ���� ����? (�̷л� �߻��ϱ� �����)
    }

    // ĳ�ÿ� ����� ��ü�� ������ ��ȯ
    return iter->second.get();
}

void ResourceManager::ReleaseAll()
{
    // (������) ��Ƽ������ �ε� �� Lock
    // std::lock_guard<std::mutex> lock(m_Mutex);
    m_TextureCache.clear(); // unique_ptr�� �ڵ����� �޸� ����
}