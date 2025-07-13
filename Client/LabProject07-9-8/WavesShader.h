#pragma once
#include "Shader.h"

class CWavesShader : public CShader
{
public:
	CWavesShader();
	virtual ~CWavesShader();

	// CShader�� ���� �Լ����� �������̵��մϴ�.
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override; // ���� ȿ���� ���� ���� ����

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

	// ���̴� Ÿ���� "Waves"�� �����մϴ�.
	virtual std::string GetShaderType() const override { return "Waves"; }

};