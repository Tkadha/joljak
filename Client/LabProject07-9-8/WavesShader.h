#pragma once
#include "Shader.h"

class CWavesShader : public CShader
{
public:
	CWavesShader();
	virtual ~CWavesShader();

	// CShader의 가상 함수들을 오버라이드합니다.
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override; // 투명 효과를 위한 블렌드 상태

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

	// 셰이더 타입을 "Waves"로 정의합니다.
	virtual std::string GetShaderType() const override { return "Waves"; }

};