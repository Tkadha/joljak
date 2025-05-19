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

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateUAVTexture2D(
        ID3D12Device* pd3dDevice, // CGameFramework로부터 전달받음
        const std::wstring& resourceName,
        UINT width, UINT height, DXGI_FORMAT format,
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        const D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    // (선택적) 생성된 리소스를 이름으로 캐싱하고 가져오는 기능
    // Microsoft::WRL::ComPtr<ID3D12Resource> GetResource(const std::wstring& name);

    // 정점/인덱스 버퍼 생성 (기존 프로젝트에 유사한 기능이 있다면 그것을 활용)
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateVertexBuffer(
        ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        const std::wstring& name, const void* pData, UINT uiElementSize, UINT uiCount, bool bIsDynamic,
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ
    );
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateIndexBuffer(
        ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        const std::wstring& name, const void* pData, UINT uiCount, // 인덱스는 보통 uint16 또는 uint32
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ
    );
    // 동적 버퍼 업데이트 함수 (필요시)
    void UpdateDynamicBuffer(ID3D12Resource* pBuffer, const void* pData, UINT uiSize);


private:
    CGameFramework* m_pFramework = nullptr;

    // 텍스처 캐시 (파일 경로 -> CTexture 객체)
    std::map<std::wstring, std::shared_ptr<CTexture>> m_TextureCache;
};