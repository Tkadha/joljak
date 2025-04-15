#pragma once

#include <d3d12.h>
#include <wrl.h> // For ComPtr
#include <string>
#include <unordered_map>
#include <memory> // For std::unique_ptr

// 필요한 클래스 전방 선언 (실제 헤더 include 필요할 수 있음)
class CTexture;
class CGameFramework;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    // 초기화 함수 (CGameFramework 포인터 또는 필요한 인터페이스 전달)
    bool Initialize(CGameFramework* pFramework);

    // 텍스처를 가져오는 주 함수 (로드 또는 캐시에서 반환)
    // 성공 시 CTexture 포인터, 실패 시 nullptr 반환
    CTexture* GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    // 모든 리소스 해제 함수
    void ReleaseAll();

private:
    // 프레임워크 (Device 및 Allocator 접근용)
    CGameFramework* m_pFramework = nullptr;
    // ID3D12Device* m_pd3dDevice = nullptr; // Framework 대신 Device만 받을 경우

    // 텍스처 캐시 (파일 경로 -> CTexture 객체)
    // unique_ptr를 사용하여 ResourceManager가 텍스처 소유권 관리
    std::unordered_map<std::wstring, std::unique_ptr<CTexture>> m_TextureCache;
};