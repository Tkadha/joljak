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

private:
    CGameFramework* m_pFramework = nullptr;

    // �ؽ�ó ĳ�� (���� ��� -> CTexture ��ü)
    std::map<std::wstring, std::shared_ptr<CTexture>> m_TextureCache;
};