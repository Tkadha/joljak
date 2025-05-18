//-----------------------------------------------------------------------------
// File: Shader.h
//-----------------------------------------------------------------------------

#pragma once

#include "Object.h"
#include "Camera.h"

class CShader
{
public:
	CShader();
	virtual ~CShader();

private:
	int									m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_RASTERIZER_DESC CreateRasterizerState();
	virtual D3D12_BLEND_DESC CreateBlendState();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader();
	virtual D3D12_SHADER_BYTECODE CreatePixelShader();

	D3D12_SHADER_BYTECODE CompileShaderFromFile(WCHAR *pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile, ID3DBlob **ppd3dShaderBlob);
	D3D12_SHADER_BYTECODE ReadCompiledShaderFromFile(WCHAR *pszFileName, ID3DBlob **ppd3dShaderBlob=NULL);

	virtual void CreateShader(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature);

	virtual void CreateShaderVariables(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList *pd3dCommandList) { }
	virtual void ReleaseShaderVariables() { }

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList *pd3dCommandList, XMFLOAT4X4 *pxmf4x4World) { }

	virtual void OnPrepareRender(ID3D12GraphicsCommandList *pd3dCommandList, int nPipelineState=0);
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList, CCamera *pCamera);

	virtual void ReleaseUploadBuffers() { }

	virtual void BuildObjects(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList, ID3D12RootSignature *pd3dGraphicsRootSignature, CLoadedModelInfo *pModel, void *pContext = NULL) { }
	virtual void AnimateObjects(float fTimeElapsed) { }
	virtual void ReleaseObjects() { }

	// --- 접근자 함수 추가 ---
   // ShaderManager 또는 CreateShader 내부에서 루트 서명을 설정하기 위한 Setter
	void SetRootSignature(ID3D12RootSignature* pRootSig) { m_pd3dGraphicsRootSignature = pRootSig; }
	// CScene::Render 등에서 사용할 Getter
	ID3D12RootSignature* GetRootSignature() const { return m_pd3dGraphicsRootSignature; }
	ID3D12PipelineState* GetPipelineState() const { return m_pd3dPipelineState; }

	// --- GetShaderType 가상 함수 추가 ---
	// 이 함수는 각 파생 셰이더 클래스가 자신의 타입을 문자열로 반환하도록 강제합니다.
	virtual std::string GetShaderType() const = 0; // 순수 가상 함수로 선언

	// 파도
	virtual void CreateComputeShader(ID3D12Device* pd3dDevice, const std::wstring& fileName, const std::string& entryPoint, ID3D12RootSignature* pd3dRootSignature);
	ID3D12PipelineState* GetComputePipelineState() { return m_pd3dComputePipelineState.Get(); }


protected:
	ID3DBlob							*m_pd3dVertexShaderBlob = NULL;
	ID3DBlob							*m_pd3dPixelShaderBlob = NULL;

	ID3D12PipelineState					*m_pd3dPipelineState = NULL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC	m_d3dPipelineStateDesc;

	float								m_fElapsedTime = 0.0f;

	// --- 루트 서명 포인터 멤버 추가 ---
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = nullptr;


	// 파도
	Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pd3dComputePipelineState;
};
