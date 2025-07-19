#pragma once
#include "Shader.h"

class CWavesShader : public CShader
{
public:
	CWavesShader();
	virtual ~CWavesShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout() override;
	virtual D3D12_BLEND_DESC CreateBlendState() override; 

	virtual D3D12_SHADER_BYTECODE CreateVertexShader() override;
	virtual D3D12_SHADER_BYTECODE CreatePixelShader() override;

	virtual std::string GetShaderType() const override { return "Waves"; }

	static ID3D12RootSignature* CreateRootSignature(ID3D12Device* pd3dDevice);
};