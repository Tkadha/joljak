#include "SkinnedShader.h"

CSkinnedAnimationStandardShader::CSkinnedAnimationStandardShader() {}
CSkinnedAnimationStandardShader::~CSkinnedAnimationStandardShader() {}

D3D12_INPUT_LAYOUT_DESC CSkinnedAnimationStandardShader::CreateInputLayout()
{
	UINT nInputElementDescs = 7;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[2] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[3] = { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[4] = { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[5] = { "BONEINDEX", 0, DXGI_FORMAT_R32G32B32A32_SINT, 5, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	pd3dInputElementDescs[6] = { "BONEWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 6, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;

	return(d3dInputLayoutDesc);
}

D3D12_SHADER_BYTECODE CSkinnedAnimationStandardShader::CreateVertexShader() {
	return(CShader::CompileShaderFromFile(L"SkinnedShaders.hlsl", "VSSkinnedAnimationStandard", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CSkinnedAnimationStandardShader::CreatePixelShader() {
	// Standard ÇÈ¼¿ ¼ÎÀÌ´õ Àç»ç¿ë
	return(CShader::CompileShaderFromFile(L"StandardShaders.hlsl", "PSStandard3", "ps_5_1", &m_pd3dPixelShaderBlob));
}

ID3D12RootSignature* CSkinnedAnimationStandardShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
	CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];
	pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 6, 0); // t6-t12
	pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 3, 0); // t3: ±×¸²ÀÚ ¸Ê SRV

	CD3DX12_ROOT_PARAMETER pd3dRootParameters[7];
	pd3dRootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // b1: Camera
	pd3dRootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);     // b2: Object
	pd3dRootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL); // b4: Lights
	pd3dRootParameters[3].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL); // t6-t12
	pd3dRootParameters[4].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL); // ±×¸²ÀÚ ¸Ê Å×ÀÌºí
	pd3dRootParameters[5].InitAsConstantBufferView(7, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b7: Bone Offsets
	pd3dRootParameters[6].InitAsConstantBufferView(8, 0, D3D12_SHADER_VISIBILITY_VERTEX); // b8: Bone Transforms

	auto d3dStaticSamplers = CShader::GetStaticSamplers();
	CD3DX12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	d3dRootSignatureDesc.Init(_countof(pd3dRootParameters), pd3dRootParameters, (UINT)d3dStaticSamplers.size(), d3dStaticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	HRESULT hr;
	ID3D12RootSignature* pd3dRootSignature = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;

	hr = D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) { if (errorBlob) OutputDebugStringA((char*)errorBlob->GetBufferPointer()); return nullptr; }
	hr = pd3dDevice->CreateRootSignature(0, signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dRootSignature));
	if (FAILED(hr)) return nullptr;
	return pd3dRootSignature;
}