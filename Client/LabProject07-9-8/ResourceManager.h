#pragma once

#include <d3d12.h>
#include <string>
#include <unordered_map>
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

    CTexture* GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    void ReleaseAll();

private:
    CGameFramework* m_pFramework = nullptr;

    // �ؽ�ó ĳ�� (���� ��� -> CTexture ��ü)
    // unique_ptr�� ����Ͽ� ResourceManager�� �ؽ�ó ������ ����
    std::unordered_map<std::wstring, std::unique_ptr<CTexture>> m_TextureCache;
};