#include "ResourceManager.h"
#include "Texture.h"       // 실제 CTexture 헤더 경로로 수정
#include "GameFramework.h" // 실제 CGameFramework 헤더 경로로 수정
#include <iostream>        // 오류 로깅 등
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


// 프리팹 등록 함수
void ResourceManager::RegisterPrefab(const std::string& name, std::shared_ptr<CGameObject> prefab)
{
    if (m_PrefabCache.find(name) == m_PrefabCache.end())
    {
        m_PrefabCache[name] = prefab;
    }
}

// 프리팹 가져오는 함수
std::shared_ptr<CGameObject> ResourceManager::GetPrefab(const std::string& name)
{
    auto findIter = m_PrefabCache.find(name);
    if (findIter != m_PrefabCache.end())
    {
        return findIter->second;
    }
    return nullptr; // 없을 경우 nullptr
}