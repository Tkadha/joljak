// TerrainShader.h
#pragma once
#include "Shader.h"

class CTerrainShader : public CShader
{
public:
    CTerrainShader();
    virtual ~CTerrainShader();

    // Input Layout 오버라이드 (지형 메쉬에 맞춤)
    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;

    // 셰이더 바이트코드 생성 오버라이드
    virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

    // 다른 상태(Rasterizer, Blend, DepthStencil)는 CShader 기본값 사용
};