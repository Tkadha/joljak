#pragma once

#include <d3d12.h>
#include <wrl.h> // For ComPtr
#include <string>
#include <unordered_map>
#include <memory> // For std::unique_ptr

// �ʿ��� Ŭ���� ���� ���� (���� ��� include �ʿ��� �� ����)
class CTexture;
class CGameFramework;

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    // �ʱ�ȭ �Լ� (CGameFramework ������ �Ǵ� �ʿ��� �������̽� ����)
    bool Initialize(CGameFramework* pFramework);

    // �ؽ�ó�� �������� �� �Լ� (�ε� �Ǵ� ĳ�ÿ��� ��ȯ)
    // ���� �� CTexture ������, ���� �� nullptr ��ȯ
    CTexture* GetTexture(const std::wstring& filename, ID3D12GraphicsCommandList* cmdList);

    // ��� ���ҽ� ���� �Լ�
    void ReleaseAll();

private:
    // �����ӿ�ũ (Device �� Allocator ���ٿ�)
    CGameFramework* m_pFramework = nullptr;
    // ID3D12Device* m_pd3dDevice = nullptr; // Framework ��� Device�� ���� ���

    // �ؽ�ó ĳ�� (���� ��� -> CTexture ��ü)
    // unique_ptr�� ����Ͽ� ResourceManager�� �ؽ�ó ������ ����
    std::unordered_map<std::wstring, std::unique_ptr<CTexture>> m_TextureCache;
};