#include "ResourceManager.h"
#include "Texture.h"       // ���� CTexture ��� ��η� ����
#include "GameFramework.h" // ���� CGameFramework ��� ��η� ����
#include "Object.h"
#include <iostream>        // ���� �α� ��
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
            throw std::runtime_error("����: CLoadedModelInfo '" + filename + "'��(��) ���Ͽ��� �ε����� ���߽��ϴ�.");
        }

        pOriginalModelInfo.reset(rawModelInfo);
        AnimateObject[filename] = pOriginalModelInfo;
    }

    if (!pOriginalModelInfo)
    {
        throw std::runtime_error("CLoadedModelInfo '" + filename + "'��(��) �ε��ϰų� ã�� �� �����ϴ�.");
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
            throw std::runtime_error("����: ���̳ʸ� ���� '" + filename + "'��(��) �� �� �����ϴ�.");
        }
        ::rewind(pInFile);

        CGameObject* loadedRawPtr = CGameObject::LoadFrameHierarchyFromFile(
            pd3dDevice, pd3dCommandList, NULL, pInFile, NULL, pGameFramework);
        ::fclose(pInFile);

        if (!loadedRawPtr) {
            throw std::runtime_error("CGameObject �ε� ����: " + filename);
        }

        pOriginalGameObject.reset(loadedRawPtr);
        StaticObject[filename] = pOriginalGameObject;
    }

    if (!pOriginalGameObject)
    {
        throw std::runtime_error("CGameObject '" + filename + "'��(��) �ε��ϰų� ã�� �� �����ϴ�.");
    }

    return *pOriginalGameObject;
}