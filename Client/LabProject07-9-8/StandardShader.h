#include "Shader.h"

class CStandardShader : public CShader
{
public:
	CStandardShader(const std::string& name);
	virtual ~CStandardShader();

	//virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	virtual std::string GetShaderType() const override { return "Standard"; }
};
