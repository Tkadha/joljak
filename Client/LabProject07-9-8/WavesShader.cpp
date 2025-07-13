#include "WavesShader.h"

CWavesShader::CWavesShader()
{
}

CWavesShader::~CWavesShader()
{
}

// 정점 데이터가 셰이더로 어떻게 입력될지 정의합니다. (stdafx.h의 Vertex 구조체와 일치)
D3D12_INPUT_LAYOUT_DESC CWavesShader::CreateInputLayout()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return d3dInputLayoutDesc;
}

// 반투명한 물을 표현하기 위한 알파 블렌딩 상태를 설정합니다.
D3D12_BLEND_DESC CWavesShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc = CShader::CreateBlendState(); // 기본 상태에서 시작
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE; // 블렌딩 활성화
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA; // 원본 색상(물)의 알파 값 사용
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // 대상 색상(배경)의 (1-알파) 값 사용
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	return d3dBlendDesc;
}


D3D12_SHADER_BYTECODE CWavesShader::CreateVertexShader()
{
	// Shaders.hlsl 파일에서 VSWaves 함수를 버텍스 셰이더로 컴파일합니다.
	return(CShader::CompileShaderFromFile(L"WavesShader.hlsl", "VS", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CWavesShader::CreatePixelShader()
{
	// Shaders.hlsl 파일에서 PSWaves 함수를 픽셀 셰이더로 컴파일합니다.
	return(CShader::CompileShaderFromFile(L"WavesShader.hlsl", "PS", "ps_5_1", &m_pd3dPixelShaderBlob));
}