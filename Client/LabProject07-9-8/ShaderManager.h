#pragma once
#include "stdafx.h"
#include <map>
#include <string>
#include <memory>
#include "Shader.h" // CShader �⺻ Ŭ���� ����

class CGameFramework; // ���� ����

class ShaderManager
{
public:
    ShaderManager(ID3D12Device* pd3dDevice);
    ~ShaderManager();

    // ���� �� ������ �����մϴ�.
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    // �����ϴ� ��� ���ҽ��� �����մϴ�.
    void ReleaseAll();

    // Ư�� �̸��� ��Ʈ �ñ״�ó�� �����ɴϴ�. ������ �����մϴ�.
    ID3D12RootSignature* GetRootSignature(const std::string& name);

    // Ư�� �̸��� ���̴��� �����ɴϴ�. ������ �����մϴ�.
    CShader* GetShader(const std::string& name);

private:
    // ��Ʈ �ñ״�ó ���� �Լ�
    void CreateRootSignatures();

    // ���̴� ���� �Լ�
    void CreateShaders();

    ID3D12Device* m_pd3dDevice;

    // ���̴��� ��Ʈ �ñ״�ó�� �̸����� �����ϴ� ��
    std::map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> m_mapRootSignatures;
    std::map<std::string, std::unique_ptr<CShader>> m_mapShaders;
};