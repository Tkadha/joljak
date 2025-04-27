// OBBShader.h
#pragma once
#include "Shader.h"

class COBBShader : public CShader
{
public:
    COBBShader();
    virtual ~COBBShader();

    // Input Layout �������̵� (Position �� ���)
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
    // Rasterizer State �������̵� (���̾�������, �ø� ����)
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

    // ���̴� ����Ʈ�ڵ� ���� �������̵�
    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

    // PSO ���� �� Primitive Topology Type�� LINE���� �����ϱ� ���� CreateShader �������̵�
    virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override;

    // �ٸ� ����(Blend, DepthStencil)�� CShader �⺻�� ���

    virtual std::string GetShaderType() const override { return "OBB"; }
};