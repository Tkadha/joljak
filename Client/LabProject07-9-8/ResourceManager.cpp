#include "ResourceManager.h"
#include "Texture.h"       // 실제 CTexture 헤더 경로로 수정
#include "GameFramework.h" // 실제 CGameFramework 헤더 경로로 수정
#include "Object.h"
#include <iostream>        // 오류 로깅 등
#include <vector>


std::unordered_map<std::string, std::shared_ptr<CLoadedModelInfo>> ResourceManager::AnimateObject;
std::unordered_map<std::string, std::shared_ptr<CGameObject>> ResourceManager::StaticObject;

std::unique_ptr<ResourceManager> ResourceManager::m_instance = nullptr;

ResourceManager* ResourceManager::GetInstance() {
    if (!m_instance) {
        m_instance = std::make_unique<ResourceManager>();
    }
    return m_instance.get();
}

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager() 
{
    ReleaseAll();
}

void ResourceManager::ReleaseAll() {
    m_TextureCache.clear();
    AnimateObject.clear();
    StaticObject.clear();
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

CLoadedModelInfo ResourceManager::GetModelInfoCopy(
    const std::string& filename,
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    CGameFramework* pGameFramework
)
{
    auto it = AnimateObject.find(filename);
    std::shared_ptr<CLoadedModelInfo> pOriginalModelInfo;

    if (it != AnimateObject.end())
    {
        pOriginalModelInfo = it->second;
    }
    else
    {
        std::vector<char> buffer(filename.length() + 1);
        filename.copy(buffer.data(), filename.length());
        buffer[filename.length()] = '\0';

        CLoadedModelInfo* rawModelInfo = CGameObject::LoadGeometryAndAnimationFromFile(
            pd3dDevice, pd3dCommandList, buffer.data(), pGameFramework);

        if (!rawModelInfo)
        {
            throw std::runtime_error("오류: CLoadedModelInfo '" + filename + "'을(를) 파일에서 로드하지 못했습니다.");
        }

        pOriginalModelInfo.reset(rawModelInfo);
        AnimateObject[filename] = pOriginalModelInfo;
    }

    if (!pOriginalModelInfo)
    {
        throw std::runtime_error("CLoadedModelInfo '" + filename + "'을(를) 로드하거나 찾을 수 없습니다.");
    }

    return *pOriginalModelInfo;
}

CGameObject ResourceManager::GetGameObjectCopy(
    const std::string& filename,
    ID3D12Device* pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    CGameFramework* pGameFramework
)
{
    auto it = StaticObject.find(filename);
    std::shared_ptr<CGameObject> pOriginalGameObject;

    if (it != StaticObject.end())
    {
        pOriginalGameObject = it->second;
    }
    else
    {

        FILE* pInFile = NULL;
        if (::fopen_s(&pInFile, filename.c_str(), "rb") != 0 || !pInFile)
        {
            throw std::runtime_error("오류: 바이너리 파일 '" + filename + "'을(를) 열 수 없습니다.");
        }
        ::rewind(pInFile);

        CGameObject* loadedRawPtr = CGameObject::LoadFrameHierarchyFromFile(
            pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
        ::fclose(pInFile);

        if (!loadedRawPtr) {
            throw std::runtime_error("CGameObject 로드 실패: " + filename);
        }

        pOriginalGameObject.reset(loadedRawPtr);
        StaticObject[filename] = pOriginalGameObject;
    }

    if (!pOriginalGameObject)
    {
        throw std::runtime_error("CGameObject '" + filename + "'을(를) 로드하거나 찾을 수 없습니다.");
    }

    return *pOriginalGameObject;
}