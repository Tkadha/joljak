#pragma once

#include <d3d12.h>
#include <string>
#include <map>
#include <unordered_map>
#include <memory> 

// 필요한 클래스 전방 선언 (실제 헤더 include 필요할 수 있음)
class CTexture;
class CGameFramework;
class CGameObject;
class CLoadedModelInfo;

class ResourceManager
{
public:
    static std::unordered_map<std::string, std::shared_ptr<CLoadedModelInfo>> AnimateObject;
    static std::unordered_map<std::string, std::shared_ptr<CGameObject>> StaticObject;

public:
    ResourceManager();
    ~ResourceManager();

    bool Initialize(CGameFramework* pFramework);

    std::shared_ptr<CTexture> GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    CLoadedModelInfo GetModelInfoCopy(const std::string& filename, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);

    CGameObject GetGameObjectCopy(const std::string& filename, ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, CGameFramework* pGameFramework);


    void ReleaseAll();

private:
    CGameFramework* m_pFramework = nullptr;

    // 텍스처 캐시 (파일 경로 -> CTexture 객체)
    std::map<std::wstring, std::shared_ptr<CTexture>> m_TextureCache;

    static std::unique_ptr<ResourceManager> m_instance;
public:
    static ResourceManager* GetInstance();
};