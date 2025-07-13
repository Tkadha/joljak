#include "Shader.h"

class CSkinnedAnimationStandardShader : public CShader
{
public:
	CSkinnedAnimationStandardShader(const std::string& name);
	virtual ~CSkinnedAnimationStandardShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual std::string GetShaderType() const override { return "Skinned"; }
};
