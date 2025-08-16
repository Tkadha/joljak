// TerrainShader.cpp
#include "TerrainShader.h"

CTerrainShader::CTerrainShader(){}

CTerrainShader::~CTerrainShader(){}

D3D12_INPUT_LAYOUT_DESC CTerrainShader::CreateInputLayout()
{
    UINT nInputElementDescs = 5;

    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    // CHeightMapGridMesh::OnPreRender 에서 설정하는 버퍼 순서와 일치해야 함
    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 0
    pd3dInputElementDescs[1] = { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 1
    pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 2 (UV0)
    pd3dInputElementDescs[3] = { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 3, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 3 (UV1)
    pd3dInputElementDescs[4] = { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 4, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }; // Slot 4

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_SHADER_BYTECODE CTerrainShader::CreateVertexShader()
{
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"TerrainShaders.hlsl", "VSTerrain", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader()
{
    // 분리된 HLSL 파일 사용
    return CompileShaderFromFile(L"TerrainShaders.hlsl", "PSTerrain", "ps_5_1", &m_pd3dPixelShaderBlob);
}

ID3D12RootSignature* CTerrainShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
    CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[2];
    pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 1, 0); // 텍스쳐 4개 
    pd3dDescriptorRanges[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 9, 0); // t9

    CD3DX12_ROOT_PARAMETER pd3dRootParameters[5]; // CBV(b1 Camera), Constants(b2 Object)
    pd3dRootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL); // 둘 다 사용
    pd3dRootParameters[1].InitAsConstants(41, 2, 0, D3D12_SHADER_VISIBILITY_ALL);     // VS에서만 필요
    pd3dRootParameters[2].InitAsConstantBufferView(4, 0, D3D12_SHADER_VISIBILITY_PIXEL);  // b4: Lights
    pd3dRootParameters[3].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요
    pd3dRootParameters[4].InitAsDescriptorTable(1, &pd3dDescriptorRanges[1], D3D12_SHADER_VISIBILITY_PIXEL); // PS에서만 필요


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