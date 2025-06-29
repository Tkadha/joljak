#pragma once

#include "Shader.h"

class CShadowShader : public CShader
{
public:
    CShadowShader();
    virtual ~CShadowShader();

    // CShader의 가상 함수들을 오버라이드합니다.
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
    virtual D3D12_SHADER_BYTECODE CreateVertexShader();
    virtual D3D12_SHADER_BYTECODE CreatePixelShader();
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState();

    // Shadow 셰이더는 깊이 값만 기록하므로, 특별한 Blend/Depth-Stencil 상태가 필요할 수 있습니다.
    virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();


    virtual std::string GetShaderType() const override { return "Shadow"; }
};