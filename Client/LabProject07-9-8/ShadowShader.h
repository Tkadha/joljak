#pragma once

#include "Shader.h"

class CShadowShader : public CShader
{
public:
    CShadowShader(const std::string& name);
    virtual ~CShadowShader();

    //virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);

    // CShader�� ���� �Լ����� �������̵��մϴ�.
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
    virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
    virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

    // Shadow ���̴��� ���� ���� ����ϹǷ�, Ư���� Blend/Depth-Stencil ���°� �ʿ��� �� �ֽ��ϴ�.
    virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();


    virtual std::string GetShaderType() const override { return "Shadow"; }
};