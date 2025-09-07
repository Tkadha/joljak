#pragma once

#include <d3d12.h>
#include <string>
#include <map>
#include <memory> 

// �ʿ��� Ŭ���� ���� ���� (���� ��� include �ʿ��� �� ����)
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

    // Prefab ���� �Լ�
    void RegisterPrefab(const std::string& name, std::shared_ptr<CGameObject> prefab);
    std::shared_ptr<CGameObject> GetPrefab(const std::string& name);

    void ReleaseAll();

private:
    CGameFramework* m_pFramework = nullptr;

    // �ؽ�ó ĳ�� (���� ��� -> CTexture ��ü)
    std::map<std::wstring, std::shared_ptr<CTexture>> m_TextureCache;

    // Prefab ����
    std::map<std::string, std::shared_ptr<CGameObject>> m_PrefabCache;
};