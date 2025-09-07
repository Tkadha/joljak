#include "WavesShader.h"

CWavesShader::CWavesShader()
{
}

CWavesShader::~CWavesShader()
{
}

// ���� �����Ͱ� ���̴��� ��� �Էµ��� �����մϴ�. (stdafx.h�� Vertex ����ü�� ��ġ)
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

// �������� ���� ǥ���ϱ� ���� ���� ���� ���¸� �����մϴ�.
D3D12_BLEND_DESC CWavesShader::CreateBlendState()
{
	D3D12_BLEND_DESC d3dBlendDesc = CShader::CreateBlendState(); // �⺻ ���¿��� ����
	d3dBlendDesc.RenderTarget[0].BlendEnable = TRUE; // ���� Ȱ��ȭ
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA; // ���� ����(��)�� ���� �� ���
	d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA; // ��� ����(���)�� (1-����) �� ���
	d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	return d3dBlendDesc;
}


D3D12_SHADER_BYTECODE CWavesShader::CreateVertexShader()
{
	// Shaders.hlsl ���Ͽ��� VSWaves �Լ��� ���ؽ� ���̴��� �������մϴ�.
	return(CShader::CompileShaderFromFile(L"WavesShader.hlsl", "VS", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CWavesShader::CreatePixelShader()
{
	// Shaders.hlsl ���Ͽ��� PSWaves �Լ��� �ȼ� ���̴��� �������մϴ�.
	return(CShader::CompileShaderFromFile(L"WavesShader.hlsl", "PS", "ps_5_1", &m_pd3dPixelShaderBlob));
}