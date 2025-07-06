#pragma once
#pragma once
#include "Shader.h"

class CDebugShader : public CShader
{
public:
    CDebugShader(const std::string& name);
    virtual ~CDebugShader();

    // Input Layout 오버라이드 (Position 만 사용)
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
    // Depth Stencil State 오버라이드 (깊이 쓰기 안 함)
    virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState() override;
    // Rasterizer State 오버라이드 (컬링 모드 변경)
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

    // 셰이더 바이트코드 생성 오버라이드
    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

    virtual std::string GetShaderType() const override { return "Debug"; }
};