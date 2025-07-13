#pragma once

#include "Shader.h"

class CLightingShader : public CShader
{
public:
    CLightingShader(const std::string& name);
    virtual ~CLightingShader();

    // CShader�� ���� �Լ����� �������̵��մϴ�.
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
    virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
    virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

    virtual std::string GetShaderType() const override { return "Lighting"; }
};