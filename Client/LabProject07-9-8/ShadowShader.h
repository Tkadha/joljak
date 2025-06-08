#pragma once

#include "Shader.h"

class CShadowShader : public CShader
{
public:
    CShadowShader();
    virtual ~CShadowShader();

    // CShader�� ���� �Լ����� �������̵��մϴ�.
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
    virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
    virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

    // Shadow ���̴��� ���� ���� ����ϹǷ�, Ư���� Blend/Depth-Stencil ���°� �ʿ��� �� �ֽ��ϴ�.
    virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();


    virtual std::string GetShaderType() const override { return "Shadow"; }
};