// ShaderManager.h

#pragma once

#include "stdafx.h"

// �̸� ���� (��ȯ ���� ����)
class CShader;

// ���� �Լ� ���� (ShaderManager.cpp ���� ����)
std::array<const CD3DX12_STATIC_SAMPLER_DESC, 3> GetStaticSamplers(); // ����: Wrap, Clamp ���÷� 2��

class ShaderManager
{
private:
    ID3D12Device* m_pd3dDevice = nullptr; // ���ҽ� ������ �ʿ�

    // ��Ʈ ���� ĳ�� (�̸� -> ��Ʈ ���� ��ü)
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> m_mapPipelineStates;
    std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12RootSignature>> m_mapRootSignatures;

    std::map<std::string, CShader*> m_Shaders;

    // ��Ʈ ���� ���� �Լ�
    ID3D12RootSignature* CreateRootSignatureInternal(const std::string& name);

    ID3D12RootSignature* CreateStandardRootSignature();
    ID3D12RootSignature* CreateSkinnedRootSignature();
    ID3D12RootSignature* CreateTerrainRootSignature();
    ID3D12RootSignature* CreateSkyboxRootSignature();
    ID3D12RootSignature* CreateOBBRootSignature();
    ID3D12RootSignature* CreateInstancingRootSignature();
    ID3D12RootSignature* CreateShadowRootSignature();
    ID3D12RootSignature* CreateDebugRootSignature();
    ID3D12RootSignature* CreateDeferedLightingRootSignature();

    // ���̴� ���� �Լ�
    CShader* CreateShaderInternal(const std::string& name, ID3D12GraphicsCommandList* pd3dCommandList);

    void CreatePSO(const std::string& name);

public:
    ShaderManager(ID3D12Device* pd3dDevice);
    ~ShaderManager(); // �Ҹ��ڿ��� ĳ�õ� ���ҽ� ���� �ʿ�

    // ����/���� ����
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    ID3D12PipelineState* GetPipelineState(const std::string& name);
    ID3D12RootSignature* GetRootSignature(const std::string& name);

    CShader* GetShader(const std::string& name, ID3D12GraphicsCommandList* pd3dCommandList);

    void ReleaseAll(); // ��� ĳ�õ� ���ҽ� ����
};