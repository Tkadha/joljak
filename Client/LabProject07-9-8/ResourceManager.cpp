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
    for (auto const& [key, pTexture] : m_TextureCache) {
        if (pTexture) pTexture->Release(); // 캐시가 가지고 있던 참조 해제
    }
    m_TextureCache.clear();
    // ... (루트 서명, 셰이더 해제) ...
}

bool ResourceManager::Initialize(CGameFramework* pFramework)
{
    if (!pFramework) return false;
    m_pFramework = pFramework;
    return true;
}

CTexture* ResourceManager::GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList)
{
    // 1. 캐시 확인
    auto findIter = m_TextureCache.find(filename);
    if (findIter != m_TextureCache.end())
    {
        // 캐시에 있으면 참조 카운트 증가 후 반환
        if (findIter->second) findIter->second->AddRef();
        return findIter->second;
    }

    // 2. 캐시에 없음 -> 새로 로드
    OutputDebugStringW((L"Loading Texture: " + filename + L"\n").c_str());

    // CTexture 객체 생성
    CTexture* newTexture = new CTexture();
    if (!newTexture) { /* 오류 처리 */ return nullptr; }

    // 3. 로드 시도
    if (!newTexture->LoadTextureFromDDSFile(m_pFramework->GetDevice(), cmdList, filename.c_str()))
    {
        OutputDebugStringW((L"Error: CTexture::LoadTextureFromDDSFile failed for: " + filename + L"\n").c_str());
        delete newTexture; // 실패 시 생성한 객체 삭제
        return nullptr;
    }

    // 4. 캐시에 추가 및 참조 카운트 관리
    newTexture->AddRef(); // 캐시에서 참조하므로 AddRef
    m_TextureCache[filename] = newTexture; // map에 저장 (대입 가능)

    newTexture->AddRef(); // 호출자를 위한 참조 AddRef
    return newTexture;
}