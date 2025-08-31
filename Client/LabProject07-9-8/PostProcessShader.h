#pragma once
#include "Shader.h"

class CPostProcessShader : public CShader
{
public:
    CPostProcessShader();
    virtual ~CPostProcessShader();

    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

    virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;
    //virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;

    static ID3D12RootSignature* CreateRootSignature(ID3D12Device* pd3dDevice);

    virtual std::string GetShaderType() const override { return "PostProcess"; }
};