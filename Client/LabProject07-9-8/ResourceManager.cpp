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