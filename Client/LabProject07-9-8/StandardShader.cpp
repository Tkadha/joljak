#include "StandardShader.h"

CStandardShader::CStandardShader(const std::string& name) { m_strShaderName = name; }
CStandardShader::~CStandardShader() {}

//void CStandardShader::CreateShader(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
//{
//	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
//	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
//	m_d3dPipelineStateDesc.VS = CreateVertexShader();
//	m_d3dPipelineStateDesc.PS = CreatePixelShader();
//	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
//	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
//	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
//	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
//	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
//	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
//	
//	if (m_strShaderName == "Standard_GBuffer")
//	{
//		// G-Buffer용 MRT
//		m_d3dPipelineStateDesc.NumRenderTargets = GBUFFER_COUNT;
//		m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
//		m_d3dPipelineStateDesc.RTVFormats[1] = DXGI_FORMAT_R16G16B16A16_FLOAT;
//		m_d3dPipelineStateDesc.RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;
//		m_d3dPipelineStateDesc.RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;
//	}
//	else // 포워드 렌더링
//	{
//		// 기존 설정 (렌더 타겟 1개)
//		m_d3dPipelineStateDesc.NumRenderTargets = 1;
//		m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
//	}
//
//
//	m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
//	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
//
//	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);
//
//	if (m_pd3dVertexShaderBlob) m_pd3dVertexShaderBlob->Release();
//	if (m_pd3dPixelShaderBlob) m_pd3dPixelShaderBlob->Release();
//
//	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs) delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
//}

D3D12_INPUT_LAYOUT_DESC CStandardShader::CreateInputLayout() {
	// ... (기존 Standard Input Layout 코드) ...
	UINT nInputElementDescs = 5;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;
	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CStandardShader::CreateVertexShader() {
	// "StandardShader.hlsl" 파일의 "VSStandard" 함수를 컴파일하도록 수정
	return(CShader::CompileShaderFromFile(L"StandardShaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CStandardShader::CreatePixelShader() {
	// "StandardShader.hlsl" 파일의 "PSStandard" 함수를 컴파일하도록 수정
	if (m_strShaderName == "Standard_GBuffer")
		return(CShader::CompileShaderFromFile(L"StandardShaders.hlsl", "PSStandard_GBuffer", "ps_5_1", &m_pd3dPixelShaderBlob));
	else
		return(CShader::CompileShaderFromFile(L"StandardShaders.hlsl", "PSStandard3", "ps_5_1", &m_pd3dPixelShaderBlob));
}
