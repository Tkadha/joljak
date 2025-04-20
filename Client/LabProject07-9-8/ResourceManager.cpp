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
    for (auto const& [key, pTexture] : m_TextureCache) {
        if (pTexture) pTexture->Release(); // ĳ�ð� ������ �ִ� ���� ����
    }
    m_TextureCache.clear();
    // ... (��Ʈ ����, ���̴� ����) ...
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
        // ĳ�ÿ� ������ ���� ī��Ʈ ���� �� ��ȯ
        if (findIter->second) findIter->second->AddRef();
        return findIter->second;
    }

    // 2. ĳ�ÿ� ���� -> ���� �ε�
    OutputDebugStringW((L"Loading Texture: " + filename + L"\n").c_str());

    // CTexture ��ü ����
    CTexture* newTexture = new CTexture();
    if (!newTexture) { /* ���� ó�� */ return nullptr; }

    // 3. �ε� �õ�
    if (!newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str()))
    {
        OutputDebugStringW((L"Error: CTexture::LoadTextureFromDDSFile failed for: " + filename + L"\n").c_str());
        delete newTexture; // ���� �� ������ ��ü ����
        return nullptr;
    }

    // 4. ĳ�ÿ� �߰� �� ���� ī��Ʈ ����
    newTexture->AddRef(); // ĳ�ÿ��� �����ϹǷ� AddRef
    m_TextureCache[filename] = newTexture; // map�� ���� (���� ����)

    newTexture->AddRef(); // ȣ���ڸ� ���� ���� AddRef
    return newTexture;
}