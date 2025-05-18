#pragma once

#include <d3d12.h>
#include <string>
#include <map>
#include <memory> 

// �ʿ��� Ŭ���� ���� ���� (���� ��� include �ʿ��� �� ����)
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
        ID3D12Device* pd3dDevice, // CGameFramework�κ��� ���޹���
        const std::wstring& resourceName,
        UINT width, UINT height, DXGI_FORMAT format,
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        const D3D12_CLEAR_VALUE* pOptimizedClearValue = nullptr,
        D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS
    );

    // (������) ������ ���ҽ��� �̸����� ĳ���ϰ� �������� ���
    // Microsoft::WRL::ComPtr<ID3D12Resource> GetResource(const std::wstring& name);

    // ����/�ε��� ���� ���� (���� ������Ʈ�� ������ ����� �ִٸ� �װ��� Ȱ��)
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateVertexBuffer(
        ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        const std::wstring& name, const void* pData, UINT uiElementSize, UINT uiCount, bool bIsDynamic,
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ
    );
    Microsoft::WRL::ComPtr<ID3D12Resource> CreateIndexBuffer(
        ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
        const std::wstring& name, const void* pData, UINT uiCount, // �ε����� ���� uint16 �Ǵ� uint32
        D3D12_RESOURCE_STATES initialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ
    );
    // ���� ���� ������Ʈ �Լ� (�ʿ��)
    void UpdateDynamicBuffer(ID3D12Resource* pBuffer, const void* pData, UINT uiSize);


private:
    CGameFramework* m_pFramework = nullptr;

    // �ؽ�ó ĳ�� (���� ��� -> CTexture ��ü)
    std::map<std::wstring, std::shared_ptr<CTexture>> m_TextureCache;
};