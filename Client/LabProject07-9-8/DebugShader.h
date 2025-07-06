#pragma once
#pragma once
#include "Shader.h"

class CDebugShader : public CShader
{
public:
    CDebugShader(const std::string& name);
    virtual ~CDebugShader();

    // Input Layout �������̵� (Position �� ���)
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
    // Depth Stencil State �������̵� (���� ���� �� ��)
    virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
    // Rasterizer State �������̵� (�ø� ��� ����)
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

    // ���̴� ����Ʈ�ڵ� ���� �������̵�
    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

    virtual std::string GetShaderType() const override { return "Debug"; }
};