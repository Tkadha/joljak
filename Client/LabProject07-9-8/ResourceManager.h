#pragma once

#include <d3d12.h>
#include <string>
#include <map>
#include <memory> 

// 필요한 클래스 전방 선언 (실제 헤더 include 필요할 수 있음)
class CTexture;
class CGameFramework;
class CGameObject;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    bool Initialize(CGameFramework* pFramework);

    std::shared_ptr<CTexture> GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    // Prefab 관리 함수
    void RegisterPrefab(const std::string& name, std::shared_ptr<CGameObject> prefab);
    std::shared_ptr<CGameObject> GetPrefab(const std::string& name);

    void ReleaseAll();

private:
    CGameFramework* m_pFramework = nullptr;

    // 텍스처 캐시 (파일 경로 -> CTexture 객체)
    std::map<std::wstring, std::shared_ptr<CTexture>> m_TextureCache;

    // Prefab 저장
    std::map<std::string, std::shared_ptr<CGameObject>> m_PrefabCache;
};