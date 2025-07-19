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


ID3D12RootSignature* CWavesShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
    CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[3];
    pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 7, 6, 0);  // t6-t12: 기본 텍스처
    pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 3, 0);  // t3: 그림자 맵
    // 변위 맵을 위한 새로운 범위를 정의합니다. (레지스터 t13 사용)
    pd3dDescriptorRanges[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13, 0);

    CD3DX12_ROOT_PARAMETER pd3dRootParameters[6];
    pd3dRootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // b1: Camera
    pd3dRootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);      // b2: GameObject
    pd3dRootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_ALL); // b4: Lights
    pd3dRootParameters[3].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
    pd3dRootParameters[4].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);
    // 변위 맵 테이블을 루트 파라미터 5번에 추가합니다. (버텍스 셰이더에서 사용)
    pd3dRootParameters[5].InitAsDescriptorTable(1, &pd3dDescriptorRanges[2], D3D12_SHADER_VISIBILITY_VERTEX);

    auto d3dStaticSamplers = CShader::GetStaticSamplers();
    CD3DX12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
    d3dRootSignatureDesc.Init(_countof(pd3dRootParameters), pd3dRootParameters, (UINT)d3dStaticSamplers.size(), d3dStaticSamplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ID3D12RootSignature* pd3dRootSignature = nullptr;
    ID3DBlob* pd3dSignatureBlob = nullptr;
    ID3DBlob* pd3dErrorBlob = nullptr;

    D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
    pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dRootSignature);

    if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
    if (pd3dErrorBlob) pd3dErrorBlob->Release();

    return pd3dRootSignature;
}