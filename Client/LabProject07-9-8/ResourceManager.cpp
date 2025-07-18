#include "ResourceManager.h"
#include "Texture.h"       // ���� CTexture ��� ��η� ����
#include "GameFramework.h" // ���� CGameFramework ��� ��η� ����
#include <iostream>        // ���� �α� ��
#include "Object.h"

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager() 
{
    ReleaseAll();
}

void ResourceManager::ReleaseAll() {
    m_TextureCache.clear();
    m_PrefabCache.clear();
}

bool ResourceManager::Initialize(CGameFramework* pFramework)
{
    if (!pFramework) return false;
    m_pFramework = pFramework;
    return true;
}

std::shared_ptr<CTexture> ResourceManager::GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList)
{
    auto findIter = m_TextureCache.find(filename);
    if (findIter != m_TextureCache.end())
    {
        return findIter->second;
    }


    auto newTexture = std::make_shared<CTexture>();
    if (!newTexture) {
        return nullptr;
    }

    if (!newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str()))
    {
        return nullptr;
    }

    m_TextureCache[filename] = newTexture; 
    return newTexture; 
}


// ������ ��� �Լ�
void ResourceManager::RegisterPrefab(const std::string& name, std::shared_ptr<CGameObject> prefab)
{
    if (m_PrefabCache.find(name) == m_PrefabCache.end())
    {
        m_PrefabCache[name] = prefab;
    }
}

// ������ �������� �Լ�
std::shared_ptr<CGameObject> ResourceManager::GetPrefab(const std::string& name)
{
    auto findIter = m_PrefabCache.find(name);
    if (findIter != m_PrefabCache.end())
    {
        return findIter->second;
    }
    return nullptr; // ���� ��� nullptr
}