#pragma once

#include <d3d12.h>
#include <string>
#include <map>
#include <memory> 

// 필요한 클래스 전방 선언 (실제 헤더 include 필요할 수 있음)
class CTexture;
class CGameFramework;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    bool Initialize(CGameFramework* pFramework);

    std::shared_ptr<CTexture> GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    void ReleaseAll();

private:
    CGameFramework* m_pFramework = nullptr;

    // 텍스처 캐시 (파일 경로 -> CTexture 객체)
    std::map<std::wstring, std::shared_ptr<CTexture>> m_TextureCache;
};