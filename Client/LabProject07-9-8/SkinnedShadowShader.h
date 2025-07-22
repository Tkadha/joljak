#pragma once
#include "SkinnedShader.h"

class CSkinnedShadowShader : public CSkinnedAnimationStandardShader
{
public:
    CSkinnedShadowShader();
    virtual ~CSkinnedShadowShader();

    // PSO 생성을 위해 일부 함수만 재정의합니다.
    //virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;
    virtual D3D12_RASTERIZER_DESC CreateRasterizerState() override;
    virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature) override;

    virtual std::string GetShaderType() const override { return "Skinned_Shadow"; }
};
