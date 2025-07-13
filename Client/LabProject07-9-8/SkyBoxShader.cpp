// SkyBoxShader.cpp
#include "SkyBoxShader.h"

CSkyBoxShader::CSkyBoxShader(){}

CSkyBoxShader::~CSkyBoxShader(){}

D3D12_INPUT_LAYOUT_DESC CSkyBoxShader::CreateInputLayout()
{
    UINT nInputElementDescs = 1;
    // ���� �Ҵ� �� CShader::CreateShader���� ������
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return d3dInputLayoutDesc;
}

D3D12_DEPTH_STENCIL_DESC CSkyBoxShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = TRUE; // ���� �׽�Ʈ�� ����
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO; // ���� ����� �� ��
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL; // ���� �׷��� ��ü �ڿ� �׷�������
    d3dDepthStencilDesc.StencilEnable = FALSE;
    d3dDepthStencilDesc.StencilReadMask = 0xFF;
    d3dDepthStencilDesc.StencilWriteMask = 0xFF;
    d3dDepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
    d3dDepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
    d3dDepthStencilDesc.BackFace = d3dDepthStencilDesc.FrontFace; // �����ϰ� ����

    return d3dDepthStencilDesc;
}

D3D12_RASTERIZER_DESC CSkyBoxShader::CreateRasterizerState()
{
    // �⺻ Rasterizer ���¸� �����ͼ� ����
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CShader::CreateRasterizerState();
    // ��ī�̹ڽ��� ���� ���� ���� �ϹǷ� �ø��� ���ų� �ո� �ø�
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_NONE; // �Ǵ� D3D12_CULL_MODE_FRONT
    return d3dRasterizerDesc;
}


D3D12_SHADER_BYTECODE CSkyBoxShader::CreateVertexShader()
{
    // �и��� HLSL ���� ���
    return CompileShaderFromFile(L"SkyboxShaders.hlsl", "VSSkyBox", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CSkyBoxShader::CreatePixelShader()
{
    // �и��� HLSL ���� ���
    return CompileShaderFromFile(L"SkyboxShaders.hlsl", "PSSkyBox", "ps_5_1", &m_pd3dPixelShaderBlob);
}

ID3D12RootSignature* CSkyBoxShader::CreateRootSignature(ID3D12Device* pd3dDevice)
{
    CD3DX12_DESCRIPTOR_RANGE pd3dDescriptorRanges[1];
    pd3dDescriptorRanges[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 13, 0); // t13: SkyCubeTexture

    CD3DX12_ROOT_PARAMETER pd3dRootParameters[2]; // CBV(b1 Camera), Table(t13 Texture)
    pd3dRootParameters[0].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_VERTEX); // VS���� View/Proj �ʿ�
    pd3dRootParameters[1].InitAsDescriptorTable(1, &pd3dDescriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL); // PS���� ���ø�

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