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