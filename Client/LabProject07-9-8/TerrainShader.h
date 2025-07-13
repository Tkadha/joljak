// TerrainShader.h
#pragma once
#include "Shader.h"

class CTerrainShader : public CShader
{
public:
    CTerrainShader(const std::string& name);
    virtual ~CTerrainShader();

    // Input Layout �������̵� (���� �޽��� ����)
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

    // ���̴� ����Ʈ�ڵ� ���� �������̵�
    virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob) override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob) override;

    // �ٸ� ����(Rasterizer, Blend, DepthStencil)�� CShader �⺻�� ���

    virtual std::string GetShaderType() const override { return "Terrain"; }
};