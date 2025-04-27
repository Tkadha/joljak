// OBBShader.h
#pragma once
#include "Shader.h"

class COBBShader : public CShader
{
public:
    COBBShader();
    virtual ~COBBShader();

    // Input Layout 오버라이드 (Position 만 사용)
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
    // Rasterizer State 오버라이드 (와이어프레임, 컬링 없음)
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;

    // 셰이더 바이트코드 생성 오버라이드
    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

    // PSO 생성 시 Primitive Topology Type을 LINE으로 설정하기 위해 CreateShader 오버라이드
    virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override;

    // 다른 상태(Blend, DepthStencil)는 CShader 기본값 사용

    virtual std::string GetShaderType() const override { return "OBB"; }
};