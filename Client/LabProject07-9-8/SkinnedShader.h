#include "Shader.h"

class CSkinnedAnimationStandardShader : public CShader
{
public:
	CSkinnedAnimationStandardShader();
	virtual ~CSkinnedAnimationStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual std::string GetShaderType() const override { return "Skinned"; }

	static ID3D12RootSignature* CreateRootSignature(ID3D12Device* pd3dDevice);
};
