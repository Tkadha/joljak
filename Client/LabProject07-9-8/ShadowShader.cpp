#include "ShadowShader.h"

CShadowShader::CShadowShader()
{
}

CShadowShader::~CShadowShader()
{
}

// �Է� ���̾ƿ�: ���� ��ġ(POSITION)�� �ʿ��մϴ�.
D3D12_INPUT_LAYOUT_DESC CShadowShader::CreateInputLayout()
{
    UINT nInputElementDescs = 1;
    D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];

    pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
    d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
    d3dInputLayoutDesc.NumElements = nInputElementDescs;

    return(d3dInputLayoutDesc);
}

// ����/���ٽ� ����: ���� �׽�Ʈ�� ���⸦ Ȱ��ȭ�մϴ�.
D3D12_DEPTH_STENCIL_DESC CShadowShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = TRUE;
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS; // �Ϲ����� ���� �׽�Ʈ
    d3dDepthStencilDesc.StencilEnable = FALSE;

    return(d3dDepthStencilDesc);
}


D3D12_SHADER_BYTECODE CShadowShader::CreateVertexShader()
{
    // "Shadow.hlsl" ������ �������մϴ�.
    //return(CShader::CompileShaderFromFile(L"StandardShaders.hlsl", "VSStandard", "vs_5_1", &m_pd3dVertexShaderBlob));

    return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "VS", "vs_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_SHADER_BYTECODE CShadowShader::CreatePixelShader()
{
    // �ȼ� ���̴��� �ƹ� �۾��� ���� �����Ƿ� NULL�� ��ȯ�մϴ�.
    return(CShader::CompileShaderFromFile(L"Shadow.hlsl", "PS", "ps_5_1", &m_pd3dVertexShaderBlob));
}

D3D12_RASTERIZER_DESC CShadowShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

    // DepthBias: ������ ����ŭ ���̸� �߰�
    d3dRasterizerDesc.DepthBias = 5000; 

    // SlopeScaledDepthBias: ǥ���� ���⿡ ���� ���̾�� �ٸ��� ����
    d3dRasterizerDesc.SlopeScaledDepthBias = 1.5f; // ���� ���ݾ� �÷�������. (��: 1.0f -> 1.5f �Ǵ� 2.0f)

    d3dRasterizerDesc.DepthBiasClamp = 0.0f;
    d3dRasterizerDesc.DepthClipEnable = TRUE;

    d3dRasterizerDesc.FrontCounterClockwise = FALSE;
    d3dRasterizerDesc.MultisampleEnable = FALSE;
    d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
    d3dRasterizerDesc.ForcedSampleCount = 0;
    d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;


    return d3dRasterizerDesc;
}